// Copyright (c) 2006  Hong Kong University of Science & Technology
// All rights reserved.
//
// This file is part of CholmodWrapper 
// (http://ihome.ust.hk/~fuhb/software.htm#cholmod_wrapper); 
// you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; version 2.1 of the License.
// See the file LICENSE.LGPL distributed with CholmodWrapper.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Author(s) : Hongbo Fu, fuplus@gmail.com, 2006

// =====================================================================	
#ifndef CHOLMOD_SOLVER_TRAITS_H
#define CHOLMOD_SOLVER_TRAITS_H

#include <iostream>

#include <fstream>
using namespace std;

#include <vector>
#include <cassert>

#include "Cholmod_dense_vector.h"
#include "Cholmod_dense_matrix.h"
#include "Sparse_coordinate_matrix.h"
#include "Cholmod_conversion.h"

/// The class Cholmod_symmetric_solver_traits
/// is a traits class for solving GENERAL (aka unsymmetric) sparse linear systems
/// using SuiteSparse cholmod solvers family.
/// 
/// Cholmod_solver_traits: Model of the SparseLinearAlgebraTraits_d concept.
///						   Model of the Direct_linear_solver concept.
/// Sparse_matrix: Model of the SparseLinearAlgebraTraits_d::Matrix concept.
/// Dense_vector: Model of the SparseLinearAlgebraTraits_d::Vector concept.
template <
	class T, 					///< Currently only double is tested.	
	class Dense_vector2 = Cholmod_dense_vector<T>,
	class Dense_matrix2 = Cholmod_dense_matrix<T>,
	class Sparse_matrix2 = Sparse_coordinate_matrix<T>
>
class Cholmod_solver_traits
{
	// -- public types -----------------------------
public:
	typedef T								NT;
	// for SparseLinearAlgebraTraits_d concept
	typedef Sparse_matrix2					Matrix;
	typedef Dense_vector2					Vector;

	typedef Sparse_matrix2					Sparse_matrix;
	typedef Dense_vector2					Dense_vector;
	typedef Dense_matrix2					Dense_matrix;

	// -- public operations ------------------------
public:
	Cholmod_solver_traits(bool verbose = false)
		: m_verbose(verbose)
		, m_symmetric(false)
		, m_num_of_unknows(0)
		, m_L(0)
		, m_At(0)
	{
		cholmod_start(&m_cholmod_common);
	}

	~Cholmod_solver_traits()
	{
		release_precomputation();
		cholmod_finish(&m_cholmod_common);
	}

	/// Solve the sparse linear system "A * X = B" in the least-squares sense.
	/// @param A the system matrix
	/// @param B the right hand side of the system. It can be either Dense_vector or Dense_matrix.
	/// @param X the final solution.
	/// @param D last entry of a homogeneous coordinate. Useless for Cholmod_solver_traits.
	/// @return true on success. 
	template <class Dense_matrix_T>
	bool linear_solver(
		const Sparse_matrix& A, const Dense_matrix_T& B, Dense_matrix_T& X, NT& D)
	{
		D = 1; // CHOLMOD does not support homogeneous coordinates		

		if (!precompute(A) && m_verbose)  // perform Cholesky factorization 
		{
			std::cerr << "FATAL ERROR: cannot PRECOMPUTE the factorization.\n";
			return false;
		}

		if (!solve(B, X) && m_verbose)	 // solve using back-substituion
		{
			std::cerr << "FATAL ERROR: cannot SOLVE the system after factorization.\n";
			return false;
		}
		return true;
	}

	template <class Dense_matrix_T>
	bool linear_solver(
		const Sparse_matrix& A, const Dense_matrix_T& B, Dense_matrix_T& X)
	{
		NT D;
		return linear_solver(A, B, X, D);
	}

	/// Peform precomputation (Cholesky decomposition) given a system matrix A.
	/// If A.is_symmetric(), A = L*L' is performed.
	/// Otherwise, A'*A = L*L' is performed. 
	/// @return true on success. 
	bool precompute(const Sparse_matrix& A)
	{
		release_precomputation(); // release old stuffs

		m_symmetric= A.is_symmetric();

		// If A is symmetric, 
		// else A*A' is analyzed.
		if (m_symmetric) // A is analyzed.
		{
			// for symmetric matrices, At = A
			m_At = create_cholmod_sparse(A, &m_cholmod_common);
		}
		else			// A*A' is analyzed: we need A'*A however.
		{
			cholmod_sparse* tmp_A = create_cholmod_sparse(A, &m_cholmod_common);
			assert(tmp_A != 0);
			m_At = cholmod_transpose(tmp_A, 1 /* array transpose */, &m_cholmod_common);
			cholmod_free_sparse(&tmp_A, &m_cholmod_common);
		}

		assert(m_At != 0);
		m_L = cholmod_analyze(m_At, &m_cholmod_common);
		if (m_L == 0) return false;

		cholmod_factorize(m_At, m_L, &m_cholmod_common);

		// number of unknowns
		m_num_of_unknows = static_cast<unsigned int>(m_At->nrow); // <==> m_A->mcol;

		return true;
	}

	/// Solve the system using back-substitution A X = B. 	
	/// X and B can be either Dense_vector or Dense_matrix. 
	/// When X and B are in Dense_matrix, it means solve 
	/// A [x1 x2 ... xn] = [b1 b2 ... bn] simultaneously. 
	/// Precondition: the system has been precomputed.
	template <class Dense_matrix_T>
	bool solve(const Dense_matrix_T& B, Dense_matrix_T& X, bool hasB = false)
	{
		if (!has_precomputed()) return false;

		cholmod_dense* b = B.create_cholmod_dense(&m_cholmod_common);
		cholmod_dense* x = 0;

		if (m_symmetric || hasB) // either Atb or b is already computed. symmetry -> // A x = b
		{
			// testing... 
			/*ofstream file("matrixb_s.txt");
			file << b->nrow << " " << b->ncol << endl;
			double* xx = (double*)b->x;
			for (int i = 0; i < (int)b->nrow; ++i)
			{
			for (int j = 0; j < (int)b->ncol; ++j)
			{
			file << i << " " << xx[i+j*b->d];
			}
			cout  << endl;
			}
			file.close();*/

			x = cholmod_solve(CHOLMOD_A, m_L, b, &m_cholmod_common);
		}
		else // A'A x = A'b
		{
			cholmod_dense* Atb = cholmod_zeros(
				m_At->nrow, b->ncol, CHOLMOD_REAL, &m_cholmod_common);
			double alpha[2] = { 1, 0 }, beta[2] = { 0, 0 };
			cholmod_sdmult(m_At, 0, alpha, beta, b, Atb, &m_cholmod_common);

			// testing... 
			/*ofstream file("matrixb.txt");
			file << Atb->nrow << " " << Atb->ncol << endl;
			double* xx = (double*)Atb->x;
			for (int i = 0; i < (int)Atb->nrow; ++i)
			{
			for (int j = 0; j < (int)Atb->ncol; ++j)
			{
			file << i << " " << xx[i+j*Atb->d];
			}
			cout << endl;
			}
			file.close();*/

			x = cholmod_solve(CHOLMOD_A, m_L, Atb, &m_cholmod_common);
		}

		assert(x != 0);
		X.assign(x);

		return true;
	}

	/// Has the system precomputed? 
	bool has_precomputed() const { return m_L != 0; }


	// --- TRUE - final pack m_L, FALSE - modify m_L later.. youyi
	void set_final_pack(int flag)
	{
		m_cholmod_common.final_pack = flag;
	}
	int is_ll()
	{
		if (m_L == NULL) return false;
		return m_L->is_ll;
	}

	// -- update/downdate factorization ----------------- added by youyi
	// -- update the factorization L by (LDL') = LDL' +/- CC' 
	bool update_factorization(const Sparse_matrix& C)
	{
		if (!has_precomputed()) return false;

		cholmod_sparse* c = create_cholmod_sparse(C, &m_cholmod_common);
		//std::cout << "create_cholmod_sparse" << std::endl;
		// permute C according to L->Perm
		cholmod_sparse* newc = cholmod_submatrix(c, (int*)m_L->Perm, m_L->n, NULL, -1, 1, 1, &m_cholmod_common);
		//std::cout << "cholmod_submatrix" << std::endl;
		if (!cholmod_updown(1, newc, m_L, &m_cholmod_common))
		{
			return false;
		}

		assert(m_L != 0);

		return true;
	}
	// -- update/downdate factorization ----------------- added by youyi
	// -- downdate the factorization L by (LDL') = LDL' +/- CC' 
	bool downdate_factorization(const Sparse_matrix& C)
	{
		if (!has_precomputed()) return false;

		cholmod_sparse* c = create_cholmod_sparse(C, &m_cholmod_common);

		// permute C according to L->Perm
		cholmod_sparse* newc = cholmod_submatrix(c, (int*)m_L->Perm, m_L->n, NULL, -1, 1, 1, &m_cholmod_common);

		if (!cholmod_updown(0, newc, m_L, &m_cholmod_common))
		{
			return false;
		}

		assert(m_L != 0);

		return true;
	}

	/// --- if the factorization m_L is changed we need to change m_At --- added by youyi
	void updateAT(const Sparse_matrix& A)
	{
		if (m_symmetric) // A is analyzed.
		{
			// for symmetric matrices, At = A
			m_At = create_cholmod_sparse(A, &m_cholmod_common);
		}
		else			// A*A' is analyzed: we need A'*A however.
		{
			cholmod_sparse* tmp_A = create_cholmod_sparse(A, &m_cholmod_common);
			assert(tmp_A != 0);

			if (m_At != NULL) cholmod_free_sparse(&m_At, &m_cholmod_common);

			m_At = cholmod_transpose(tmp_A, 1 /* array transpose */, &m_cholmod_common);
			cholmod_free_sparse(&tmp_A, &m_cholmod_common);
		}
	}


	// -- private operations -----------------------
private:
	void release_precomputation()
	{
		// it is safe to release NULL pointers in cholmod.
		
			cholmod_free_factor(&m_L, &m_cholmod_common);
			cholmod_free_sparse(&m_At, &m_cholmod_common);
	}

	// -- private variables ------------------------
private:

	cholmod_sparse* m_At;


	bool m_symmetric;
	bool m_verbose;
	unsigned int m_num_of_unknows;
public:
	cholmod_factor* m_L;
	cholmod_common m_cholmod_common;
};

#endif // CHOLMOD_SOLVER_TRAITS_H
// =====================================================================	