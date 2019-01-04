#include "LayoutRefiner.h"
CLayoutRefiner::CLayoutRefiner(int shape_size) :my_solver_(NULL)
{
	shape_num_ = shape_size;
}

CLayoutRefiner::CLayoutRefiner() : my_solver_(NULL)
{
	shape_num_ = 0;
}

CLayoutRefiner::~CLayoutRefiner()
{

}
void CLayoutRefiner::setmatrixM(double m1[8])
{
	for (int i = 0; i < 8; i++)
	{
		m_[i] = m1[i];
	}
}

void CLayoutRefiner::setFirst3DPosition(cv::Vec3d point, int index)
{
	first_3d_position_.push_back(point);
	first_3d_position_location.push_back(index);
}

void CLayoutRefiner::setShapeSize(cv::Vec3d shape_size)
{
	shapes_size_.push_back(shape_size);
}

void CLayoutRefiner::computeShapeAndTransformatMatrix()
{
	//cv::Mat matA = cv::Mat::zeros(numConstraint, numVariable, CV_64F);
	transformM_ = cv::Mat::zeros(shape_num_ * 3, shape_num_ * 3, CV_64F);
	shape_sizeM_ = cv::Mat::zeros(shape_num_ * 3, shape_num_ * 3, CV_64F);

	double matrixMM[3][3];
	matrixMM[0][0] = m_[0] * m_[0] + m_[4] * m_[4]; matrixMM[0][1] = m_[0] * m_[1] + m_[4] * m_[5]; matrixMM[0][2] = m_[0] * m_[2] + m_[4] * m_[6];
	matrixMM[1][0] = m_[0] * m_[1] + m_[4] * m_[5]; matrixMM[1][1] = m_[1] * m_[1] + m_[5] * m_[5]; matrixMM[1][2] = m_[1] * m_[2] + m_[5] * m_[6];
	matrixMM[2][0] = m_[0] * m_[2] + m_[4] * m_[6]; matrixMM[2][1] = m_[1] * m_[2] + m_[5] * m_[6]; matrixMM[2][2] = m_[2] * m_[2] + m_[6] * m_[6];

	for (int i = 0; i < shape_num_; i++)
	{
		transformM_.at<double>(0 + i * 3, 0 + i * 3) = matrixMM[0][0];	transformM_.at<double>(0 + i * 3, 1 + i * 3) = matrixMM[0][1];	transformM_.at<double>(0 + i * 3, 2 + i * 3) = matrixMM[0][2];
		transformM_.at<double>(1 + i * 3, 0 + i * 3) = matrixMM[1][0];  transformM_.at<double>(1 + i * 3, 1 + i * 3) = matrixMM[1][1]; transformM_.at<double>(1 + i * 3, 2 + i * 3) = matrixMM[1][2];
		transformM_.at<double>(2 + i * 3, 0 + i * 3) = matrixMM[2][0];  transformM_.at<double>(2 + i * 3, 1 + i * 3) = matrixMM[2][1]; transformM_.at<double>(2 + i * 3, 2 + i * 3) = matrixMM[2][2];
	}
	for (int i = 0; i < shape_num_ * 3; i++)
	{
		shape_sizeM_.at<double>(i, i) = 20000;//1 size
	}
	//固定shape的size约束乘以2000
	for (int i = 0; i < first_3d_position_location.size(); i++)
	{
		int no = first_3d_position_location[i];
		shape_sizeM_.at<double>(no * 3 + 0, no * 3 + 0) = 20000;//1 
		shape_sizeM_.at<double>(no * 3 + 1, no * 3 + 1) = 20000;//
		shape_sizeM_.at<double>(no * 3 + 2, no * 3 + 2) = 20000;//
	}
	//
	upper_part_matrixA_ = cv::Mat::zeros(shape_num_ * 6 + 3 * first_3d_position_location.size(), shape_num_ * 6, CV_64F);
	for (int i = 0; i < shape_num_ * 3; i++)
	{
		for (int j = 0; j < shape_num_ * 3; j++)
		{
			upper_part_matrixA_.at<double>(i, j) = transformM_.at<double>(i, j);

		}
	}

	//
	for (int i = 0; i < shape_num_ * 3; i++)
	{
		for (int j = 0; j < shape_num_ * 3; j++)
		{
			upper_part_matrixA_.at<double>(i + shape_num_ * 3, j + shape_num_ * 3) = shape_sizeM_.at<double>(i, j);
		}
	}
	for (int i = 0; i < first_3d_position_location.size(); i++)
	{
		int no = first_3d_position_location[i];
		upper_part_matrixA_.at<double>(shape_num_ * 6 + i * 3, no * 3 + 0) = 20000;//1 20000
		upper_part_matrixA_.at<double>(shape_num_ * 6 + i * 3 + 1, no * 3 + 1) = 20000;//
		upper_part_matrixA_.at<double>(shape_num_ * 6 + i * 3 + 2, no * 3 + 2) = 20000;//
	}





}

void CLayoutRefiner::computeConstraints()
{
	for (int p = 0; p < constraint_dir.size(); p++)
	{
		cv::Mat constraints_mat_ = cv::Mat::zeros(3, shape_num_ * 6, CV_64F);
		double vx = constraint_dir[p][0];
		double vy = constraint_dir[p][1];
		double vz = constraint_dir[p][2];
		for (int i = 0; i < 3; i++)
		{
			if (i % 3 == 0){ constraints_mat_.at<double>(i, 0) = -vy; constraints_mat_.at<double>(i, 1) = vx; }
			if (i % 3 == 1){ constraints_mat_.at<double>(i, 0) = -vz; constraints_mat_.at<double>(i, 2) = vx; }
			if (i % 3 == 2) { constraints_mat_.at<double>(i, 1) = -vz; constraints_mat_.at<double>(i, 2) = vy; }

		}
		constraints_mat_.at<double>(0, 3) = vy; constraints_mat_.at<double>(0, 4) = -vx;
		constraints_mat_.at<double>(1, 3) = vz; constraints_mat_.at<double>(1, 5) = -vx;
		constraints_mat_.at<double>(2, 4) = vz; constraints_mat_.at<double>(2, 5) = -vy;

		//TransformConstraint
		double m[9];
		for (int i = 0; i < 9; i++)
		{
			m[i] = reference_Matrix[p * 2][i];
		}

		for (int i = 0; i < 3; i++)
		{
			if (i % 3 == 0){ constraints_mat_.at<double>(i, 6) = m[3] * vx - m[0] * vy; constraints_mat_.at<double>(i, 7) = m[4] * vx - m[1] * vy; constraints_mat_.at<double>(i, 8) = m[5] * vx - m[2] * vy; }
			if (i % 3 == 1){ constraints_mat_.at<double>(i, 6) = m[6] * vx - m[0] * vz; constraints_mat_.at<double>(i, 7) = m[7] * vx - m[1] * vz; constraints_mat_.at<double>(i, 8) = m[8] * vx - m[2] * vz; }
			if (i % 3 == 2) { constraints_mat_.at<double>(i, 6) = m[6] * vy - m[3] * vz; constraints_mat_.at<double>(i, 7) = m[7] * vy - m[4] * vz; constraints_mat_.at<double>(i, 8) = m[8] * vy - m[5] * vz; }
		}
		for (int i = 0; i < shape_num_ - 1; i++)
		{
			for (int k = 0; k < 9; k++)
			{
				m[k] = reference_Matrix[p * 2 + 1][k];
			}
			constraints_mat_.at<double>(0, 9) = m[0] * vy - m[3] * vx; constraints_mat_.at<double>(0, 10) = m[1] * vy - m[4] * vx; constraints_mat_.at<double>(0, 11) = m[2] * vy - m[5] * vx;
			constraints_mat_.at<double>(1, 9) = m[0] * vz - m[6] * vx; constraints_mat_.at<double>(1, 10) = m[1] * vz - m[7] * vx; constraints_mat_.at<double>(1, 11) = m[2] * vz - m[8] * vx;
			constraints_mat_.at<double>(2, 9) = m[3] * vz - m[6] * vy; constraints_mat_.at<double>(2, 10) = m[4] * vz - m[7] * vy; constraints_mat_.at<double>(2, 11) = m[5] * vz - m[8] * vy;
		}
		constraints_mats_.push_back(constraints_mat_);
	}
}
void CLayoutRefiner::computeConstraints2()
{
	double m[9];
	int no = -1;
	for (int p = 0; p < constraint_dir.size(); p++)
	{
		double vx = constraint_dir[p][0];
		double vy = constraint_dir[p][1];
		double vz = constraint_dir[p][2];
		vector<vector<double>> references = reference_Matrix_vector[p];
		for (int t = 1; t < references.size(); t++)
		{
			cv::Mat constraints_mat_ = cv::Mat::zeros(3, shape_num_ * 6, CV_64F);

			//TransformConstraint
			for (int i = 0; i < 9; i++)
			{
				m[i] = references[t - 1][i];
				//	cout << "  " << m[i];
			}
			//	cout << "\n";
			no = references[t - 1][9];
			for (int i = 0; i < 3; i++)
			{
				if (i % 3 == 0){ constraints_mat_.at<double>(i, shape_num_ * 3 + no * 3 + 0) = m[3] * vx - m[0] * vy; constraints_mat_.at<double>(i, shape_num_ * 3 + no * 3 + 1) = m[4] * vx - m[1] * vy; constraints_mat_.at<double>(i, shape_num_ * 3 + no * 3 + 2) = m[5] * vx - m[2] * vy; }
				if (i % 3 == 1){ constraints_mat_.at<double>(i, shape_num_ * 3 + no * 3 + 0) = m[6] * vx - m[0] * vz; constraints_mat_.at<double>(i, shape_num_ * 3 + no * 3 + 1) = m[7] * vx - m[1] * vz; constraints_mat_.at<double>(i, shape_num_ * 3 + no * 3 + 2) = m[8] * vx - m[2] * vz; }
				if (i % 3 == 2) { constraints_mat_.at<double>(i, shape_num_ * 3 + no * 3 + 0) = m[6] * vy - m[3] * vz; constraints_mat_.at<double>(i, shape_num_ * 3 + no * 3 + 1) = m[7] * vy - m[4] * vz; constraints_mat_.at<double>(i, shape_num_ * 3 + no * 3 + 2) = m[8] * vy - m[5] * vz; }
			}

			for (int i = 0; i < 3; i++)
			{
				if (i % 3 == 0){ constraints_mat_.at<double>(i, no * 3 + 0) = -vy; constraints_mat_.at<double>(i, no * 3 + 1) = vx; }
				if (i % 3 == 1){ constraints_mat_.at<double>(i, no * 3 + 0) = -vz; constraints_mat_.at<double>(i, no * 3 + 2) = vx; }
				if (i % 3 == 2) { constraints_mat_.at<double>(i, no * 3 + 1) = -vz; constraints_mat_.at<double>(i, no * 3 + 2) = vy; }

			}



			for (int i = 0; i < 9; i++)
			{
				m[i] = references[t][i];
				//cout << "  " << m[i];
			}
			//cout << "\n";
			no = references[t][9];
			constraints_mat_.at<double>(0, no * 3 + 0) = vy; constraints_mat_.at<double>(0, no * 3 + 1) = -vx;
			constraints_mat_.at<double>(1, no * 3 + 0) = vz; constraints_mat_.at<double>(1, no * 3 + 2) = -vx;
			constraints_mat_.at<double>(2, no * 3 + 1) = vz; constraints_mat_.at<double>(2, no * 3 + 2) = -vy;

			constraints_mat_.at<double>(0, shape_num_ * 3 + no * 3 + 0) = m[0] * vy - m[3] * vx; constraints_mat_.at<double>(0, shape_num_ * 3 + no * 3 + 1) = m[1] * vy - m[4] * vx; constraints_mat_.at<double>(0, shape_num_ * 3 + no * 3 + 2) = m[2] * vy - m[5] * vx;
			constraints_mat_.at<double>(1, shape_num_ * 3 + no * 3 + 0) = m[0] * vz - m[6] * vx; constraints_mat_.at<double>(1, shape_num_ * 3 + no * 3 + 1) = m[1] * vz - m[7] * vx; constraints_mat_.at<double>(1, shape_num_ * 3 + no * 3 + 2) = m[2] * vz - m[8] * vx;
			constraints_mat_.at<double>(2, shape_num_ * 3 + no * 3 + 0) = m[3] * vz - m[6] * vy; constraints_mat_.at<double>(2, shape_num_ * 3 + no * 3 + 1) = m[4] * vz - m[7] * vy; constraints_mat_.at<double>(2, shape_num_ * 3 + no * 3 + 2) = m[5] * vz - m[8] * vy;
			constraints_mats_.push_back(constraints_mat_);
		}

	}
}
cv::Mat CLayoutRefiner::getUpperPartMatrixA()
{
	computeShapeAndTransformatMatrix();
	return upper_part_matrixA_;
}

void CLayoutRefiner::setPosition2D(cv::Vec2d point)
{
	point_2d.push_back(point);
}

void CLayoutRefiner::computeMatB()
{
	mat_B = cv::Mat::zeros(upper_part_matrixA_.rows + constraints_mats_.size() * 3 + spacingConstraints_mats_.size(), 1, CV_64F);
	for (int i = 0; i < point_2d.size(); i++)
	{
		double a = point_2d[i][0];
		double b = point_2d[i][1];
		double vec_b[3];
		vec_b[0] = a*m_[0] - m_[0] * m_[3] + b*m_[4] - m_[4] * m_[7];
		vec_b[1] = a*m_[1] - m_[1] * m_[3] + b*m_[5] - m_[5] * m_[7];
		vec_b[2] = a*m_[2] - m_[2] * m_[3] + b*m_[6] - m_[6] * m_[7];
		mat_B.at<double>(i * 3 + 0, 0) = vec_b[0];
		mat_B.at<double>(i * 3 + 1, 0) = vec_b[1];
		mat_B.at<double>(i * 3 + 2, 0) = vec_b[2];
	}
	for (int i = 0; i < shape_num_; i++)
	{
		mat_B.at<double>(shape_num_ * 3 + i * 3 + 0, 0) = shapes_size_[i][0]*20000;
		mat_B.at<double>(shape_num_ * 3 + i * 3 + 1, 0) = shapes_size_[i][1] * 20000;
		mat_B.at<double>(shape_num_ * 3 + i * 3 + 2, 0) = shapes_size_[i][2] * 20000;
	}
	//shape size约束乘以2000
	for (int i = 0; i < first_3d_position_.size(); i++)
	{
		int no = first_3d_position_location[i];
		mat_B.at<double>(shape_num_ * 3 + no * 3 + 0, 0) = mat_B.at<double>(shape_num_ * 3 + no * 3 + 0, 0);
		mat_B.at<double>(shape_num_ * 3 + no * 3 + 1, 0) = mat_B.at<double>(shape_num_ * 3 + no * 3 + 1, 0);
		mat_B.at<double>(shape_num_ * 3 + no * 3 + 2, 0) = mat_B.at<double>(shape_num_ * 3 + no * 3 + 2, 0);
	}
	//
	for (int i = 0; i < first_3d_position_.size(); i++)
	{
		mat_B.at<double>(6 * shape_num_ + i * 3 + 0, 0) = first_3d_position_[i][0]*20000;//
		mat_B.at<double>(6 * shape_num_ + i * 3 + 1, 0) = first_3d_position_[i][1]*20000;
		mat_B.at<double>(6 * shape_num_ + i * 3 + 2, 0) = first_3d_position_[i][2]*20000;
	}

}

cv::Mat CLayoutRefiner::getMatB()
{
	computeMatB();
	return mat_B;
}

void CLayoutRefiner::setReferenceMatrix(vector<vector<double>> matrix)
{
	reference_Matrix_vector.push_back(matrix);
	/*
	vector<double> vec;
	for (int i = 0; i < 9; i++)
	{
	vec.push_back(m1[i]);
	}
	reference_Matrix.push_back(vec);
	*/
}

void CLayoutRefiner::setConstraint_dir(cv::Vec3d dir)
{
	constraint_dir.push_back(dir);
}

cv::Mat CLayoutRefiner::getMatA()
{
	computeShapeAndTransformatMatrix();
	computeConstraints2();
	mat_A = cv::Mat::zeros(upper_part_matrixA_.rows + constraints_mats_.size() * 3, shape_num_ * 6, CV_64F);
	for (int i = 0; i < upper_part_matrixA_.rows; i++)
	{
		for (int j = 0; j < shape_num_ * 6; j++)
		{
			mat_A.at<double>(i, j) = upper_part_matrixA_.at<double>(i, j);
		}
	}
	for (int k = 0; k < constraints_mats_.size(); k++)
	{
		cv::Mat constraints_mat_ = constraints_mats_[k];
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < shape_num_ * 6; j++)
			{
				mat_A.at<double>(k * 3 + i + upper_part_matrixA_.rows, j) = 20000 * constraints_mat_.at<double>(i, j);
			}
		}
	}
	return mat_A;
}

void CLayoutRefiner::SetData(CShapeHandler* shapeHandlerp)
{
	m_shapeHandler_h = shapeHandlerp;
}

void CLayoutRefiner::RotateWithOneRelaion(Edge edge_from, Edge edge_to)
{
	cv::Vec3d dir_to, dir_from;

	CShapePart* activePart = m_shapeHandler_h->m_allParts[edge_from.m_shapeIndex];
	qglviewer::Vec localPos_from, localPos_to;
	localPos_from = Point2Direction(edge_from.m_globalPt3D, activePart->m_frame);
	localPos_to = Point2Direction(edge_to.m_globalPt3D, activePart->m_frame);
	dir_from = Vec2CVec(localPos_from);
	dir_to = Vec2CVec(localPos_to);
	if (dir_from.dot(dir_to) < 0)
	{
		localPos_to = -localPos_to;
	}
	qglviewer::Quaternion q = GetAxisAngleBy2Vector(localPos_from, localPos_to);
	activePart->m_frame->rotate(q);
}

qglviewer::Vec CLayoutRefiner::Point2Direction(cv::Vec3d endPt3D[2], qglviewer::ManipulatedFrame* frame)
{
	qglviewer::Vec localPos_from, localPos_to, from_point1, from_point2;
	from_point1[0] = endPt3D[0][0]; from_point1[1] = endPt3D[0][1]; from_point1[2] = endPt3D[0][2];
	from_point2[0] = endPt3D[1][0]; from_point2[1] = endPt3D[1][1]; from_point2[2] = endPt3D[1][2];
	from_point1 = frame->localCoordinatesOf(from_point1);
	from_point2 = frame->localCoordinatesOf(from_point2);
	localPos_from = from_point2 - from_point1;
	localPos_from.normalize();
	return localPos_from;
}

cv::Vec3d CLayoutRefiner::Vec2CVec(qglviewer::Vec v)
{
	cv::Vec3d vec;
	vec[0] = v[0];
	vec[1] = v[1];
	vec[2] = v[2];
	return vec;
}

qglviewer::Quaternion CLayoutRefiner::GetAxisAngleBy2Vector(qglviewer::Vec from, qglviewer::Vec to)
{
	const qreal epsilon = 1E-10;

	const qreal fromSqNorm = from.squaredNorm();
	const qreal toSqNorm = to.squaredNorm();
	// Identity Quaternion when one vector is null
	if ((fromSqNorm < epsilon) || (toSqNorm < epsilon)) {
		qglviewer::Quaternion q;
		q[0] = q[1] = q[2] = 0.0;
		q[3] = 1.0;
		return q;
	}
	else
	{
		qglviewer::Vec axis = cross(from, to);
		const qreal axisSqNorm = axis.squaredNorm();

		// Aligned vectors, pick any axis, not aligned with from or to
		if (axisSqNorm < epsilon)
			axis = from.orthogonalVec();

		qreal angle = asin(sqrt(axisSqNorm / (fromSqNorm * toSqNorm)));

		if (from * to < 0.0)
			angle = M_PI - angle;
		return qglviewer::Quaternion(axis, angle*Utils::gdv_RotatingRatio);
	}
}

void CLayoutRefiner::RotateWithTwoRelaion(Edge edge_from, Edge edge_to)
{
	cv::Vec3d dir_to, dir_from, dir_to1;
	qglviewer::Vec localPos_from, localPos_to, localPos_to1, axis;

	CShapePart* activePart = m_shapeHandler_h->m_allParts[edge_from.m_shapeIndex];
	axis = Point2Direction(edge_from.m_globalPt3D, activePart->m_frame);
	Edge e = activePart->edges[(edge_from.m_shapeEdgeNo + 1) % 12];
	localPos_from = Point2Direction(e.m_globalPt3D, activePart->m_frame);
	CShapePart* activePart1 = m_shapeHandler_h->m_allParts[edge_to.m_shapeIndex];
	e = activePart1->edges[(edge_to.m_shapeEdgeNo + 1) % 12];
	localPos_to = Point2Direction(e.m_globalPt3D, activePart->m_frame);
	e = activePart1->edges[(edge_to.m_shapeEdgeNo + 2) % 12];
	localPos_to1 = Point2Direction(e.m_globalPt3D, activePart->m_frame);
	dir_from = Vec2CVec(localPos_from);
	dir_to = Vec2CVec(localPos_to);
	dir_to1 = Vec2CVec(localPos_to1);
	if (abs(dir_from.dot(dir_to)) < abs(dir_from.dot(dir_to1)))
	{
		dir_to = dir_to1;
		localPos_to = localPos_to1;
	}
	if (dir_from.dot(dir_to) < 0)
	{
		localPos_to = -localPos_to;
	}
	qglviewer::Quaternion q = GetAxisAngleBy2Vector(localPos_from, localPos_to);
	activePart->m_frame->rotate(q);

}

void CLayoutRefiner::UpdateVecRelations(vector<ShapeRelation> &availableRelations)
{
	CShapePart* activePart1;
	CShapePart* activePart2;
	vector<ShapeRelation> relations;
	for (int i = 0; i < availableRelations.size(); i++)
	{
		relations.push_back(availableRelations[i]);
	}
	availableRelations.clear();
	for (int i = 0; i < relations.size(); i++)
	{
		ShapeRelation shaper;
		Edge e1, e2, ee1, ee2;
		e1 = relations[i].edges[0];
		e2 = relations[i].edges[1];
		activePart1 = m_shapeHandler_h->m_allParts[e1.m_shapeIndex];
		activePart2 = m_shapeHandler_h->m_allParts[e2.m_shapeIndex];
		ee1 = activePart1->edges[e1.m_shapeEdgeNo];
		ee2 = activePart2->edges[e2.m_shapeEdgeNo];
		ee1.m_shapeIndex = e1.m_shapeIndex;
		ee2.m_shapeIndex = e2.m_shapeIndex;
		shaper.edges[0] = ee1;
		shaper.edges[1] = ee2;
		shaper.is2relations = relations[i].is2relations;
		shaper.recordNo = relations[i].recordNo;
		availableRelations.push_back(shaper);
	}
}

void CLayoutRefiner::OneCoordinateAlignment(Edge edge_from, Edge edge_to)
{
	CShapePart* activePart_form = m_shapeHandler_h->m_allParts[edge_from.m_shapeIndex];
	CShapePart* activePart_to = m_shapeHandler_h->m_allParts[edge_to.m_shapeIndex];
	cv::Vec3d from_x, from_y, from_z, to_x, to_y, to_z;
	cv::Vec3d dir_from = edge_from.m_dir3D;
	double m_from[16], m_to[16];
	activePart_form->m_frame->getMatrix(m_from);
	activePart_to->m_frame->getMatrix(m_to);
	from_x[0] = m_from[0]; from_x[1] = m_from[1]; from_x[2] = m_from[2];
	from_y[0] = m_from[4]; from_y[1] = m_from[5]; from_y[2] = m_from[6];
	from_z[0] = m_from[8]; from_z[1] = m_from[9]; from_z[2] = m_from[10];


	to_x[0] = m_to[0]; to_x[1] = m_to[1]; to_x[2] = m_to[2];
	to_y[0] = m_to[4]; to_y[1] = m_to[5]; to_y[2] = m_to[6];
	to_z[0] = m_to[8]; to_z[1] = m_to[9]; to_z[2] = m_to[10];
	double dis_max, dis_x, dis_y, dis_z;
	cv::Vec3d mat_temp;
	//判断dir和哪个坐标轴对齐
	dis_x = abs(dir_from.dot(from_x));
	dis_y = abs(dir_from.dot(from_y));
	dis_z = abs(dir_from.dot(from_z));
	dis_max = dis_x;
	if (dis_y > dis_max)dis_max = dis_y;
	if (dis_z > dis_max)dis_max = dis_z;
	if (dis_x == dis_max)
	{
		dis_x = abs(from_x.dot(to_x));
		dis_y = abs(from_x.dot(to_y));
		dis_z = abs(from_x.dot(to_z));
		dis_max = dis_x;
		if (dis_y > dis_max)dis_max = dis_y;
		if (dis_z > dis_max)dis_max = dis_z;
		if (dis_x == dis_max)
		{
			if (from_x.dot(to_x) > 0)
			{
				from_x = to_x;
			}
			else
			{
				from_x = -to_x;
			}
		}
		else if (dis_y == dis_max)
		{
			if (from_x.dot(to_y) > 0)
			{
				from_x = to_y;
			}
			else
			{
				from_x = -to_y;
			}
		}
		else if (dis_z == dis_max)
		{
			if (from_x.dot(to_z) > 0)
			{
				from_x = to_z;
			}
			else
			{
				from_x = -to_z;
			}
		}
		/*	mat_temp = from_x.cross(from_y);
		if (mat_temp.dot(from_z) > 0)
		{
		from_z = mat_temp;
		}
		else
		{
		from_z = -mat_temp;
		}
		from_z = Normalize(from_z);
		mat_temp = from_x.cross(from_z);
		if (mat_temp.dot(from_y) > 0)
		{
		from_y = mat_temp;
		}
		else
		{
		from_y = -mat_temp;
		}
		from_y = Normalize(from_y);*/
	}
	else if (dis_y == dis_max)
	{
		dis_x = abs(from_y.dot(to_x));
		dis_y = abs(from_y.dot(to_y));
		dis_z = abs(from_y.dot(to_z));
		dis_max = dis_x;
		if (dis_y > dis_max)dis_max = dis_y;
		if (dis_z > dis_max)dis_max = dis_z;
		if (dis_x == dis_max)
		{
			if (from_y.dot(to_x) > 0)
			{
				from_y = to_x;
			}
			else
			{
				from_y = -to_x;
			}
		}
		else if (dis_y == dis_max)
		{
			if (from_y.dot(to_y) > 0)
			{
				from_y = to_y;
			}
			else
			{
				from_y = -to_y;
			}
		}
		else if (dis_z == dis_max)
		{
			if (from_y.dot(to_z) > 0)
			{
				from_y = to_z;
			}
			else
			{
				from_y = -to_z;
			}
		}
		/*mat_temp = from_y.cross(from_x);
		if (mat_temp.dot(from_z) > 0)
		{
		from_z = mat_temp;
		}
		else
		{
		from_z = -mat_temp;
		}
		from_z = Normalize(from_z);
		mat_temp = from_y.cross(from_z);
		if (mat_temp.dot(from_y) > 0)
		{
		from_x = mat_temp;
		}
		else
		{
		from_x = -mat_temp;
		}
		from_x = Normalize(from_x);*/
	}
	else if (dis_z == dis_max)
	{
		dis_x = abs(from_z.dot(to_x));
		dis_y = abs(from_z.dot(to_y));
		dis_z = abs(from_z.dot(to_z));
		dis_max = dis_x;
		if (dis_y > dis_max)dis_max = dis_y;
		if (dis_z > dis_max)dis_max = dis_z;
		if (dis_x == dis_max)
		{
			if (from_z.dot(to_x) > 0)
			{
				from_z = to_x;
			}
			else
			{
				from_z = -to_x;
			}
		}
		else if (dis_y == dis_max)
		{
			if (from_z.dot(to_y) > 0)
			{
				from_z = to_y;
			}
			else
			{
				from_z = -to_y;
			}
		}
		else if (dis_z == dis_max)
		{
			if (from_z.dot(to_z) > 0)
			{
				from_z = to_z;
			}
			else
			{
				from_z = -to_z;
			}
		}
		/*mat_temp = from_z.cross(from_y);
		if (mat_temp.dot(from_x) > 0)
		{
		from_x = mat_temp;
		}
		else
		{
		from_x = -mat_temp;
		}
		from_x = Normalize(from_x);
		mat_temp = from_z.cross(from_x);
		if (mat_temp.dot(from_y) > 0)
		{
		from_y = mat_temp;
		}
		else
		{
		from_y = -mat_temp;
		}
		from_y = Normalize(from_y);*/
	}

	//
	m_from[0] = from_x[0];  m_from[1] = from_x[1];  m_from[2] = from_x[2];
	m_from[4] = from_y[0];	m_from[5] = from_y[1];  m_from[6] = from_y[2];
	m_from[8] = from_z[0];  m_from[9] = from_z[1];  m_from[10] = from_z[2];
	activePart_form->m_frame->setFromMatrix(m_from);
}

void CLayoutRefiner::AllCoordinateAlignment(Edge edge_from, Edge edge_to)
{
	
	CShapePart* activePart_form = m_shapeHandler_h->m_allParts[edge_from.m_shapeIndex];
	CShapePart* activePart_to = m_shapeHandler_h->m_allParts[edge_to.m_shapeIndex];
	cv::Vec3d from_x, from_y, from_z, to_x, to_y, to_z;
	if (activePart_form==NULL)
	{
		return;
	}
	if (activePart_to==NULL)
	{
		return;
	}

	double m_from[16], m_to[16];
	activePart_form->m_frame->getMatrix(m_from);
	activePart_to->m_frame->getMatrix(m_to);
	from_x[0] = m_from[0]; from_x[1] = m_from[1]; from_x[2] = m_from[2];
	from_y[0] = m_from[4]; from_y[1] = m_from[5]; from_y[2] = m_from[6];
	from_z[0] = m_from[8]; from_z[1] = m_from[9]; from_z[2] = m_from[10];
	to_x[0] = m_to[0]; to_x[1] = m_to[1]; to_x[2] = m_to[2];
	to_y[0] = m_to[4]; to_y[1] = m_to[5]; to_y[2] = m_to[6];
	to_z[0] = m_to[8]; to_z[1] = m_to[9]; to_z[2] = m_to[10];
	double dis_max, dis_x, dis_y, dis_z;
	//x轴对齐
	dis_x = abs(from_x.dot(to_x));
	dis_y = abs(from_x.dot(to_y));
	dis_z = abs(from_x.dot(to_z));
	dis_max = dis_x;
	if (dis_y > dis_max)dis_max = dis_y;
	if (dis_z > dis_max)dis_max = dis_z;
	if (dis_x == dis_max)
	{
		if (from_x.dot(to_x)>0)
		{
			from_x = to_x;
		}
		else
		{
			from_x = -to_x;
		}
	}
	if (dis_y == dis_max)
	{
		if (from_x.dot(to_y) > 0)
		{
			from_x = to_y;
		}
		else
		{
			from_x = -to_y;
		}
	}
	if (dis_z == dis_max)
	{
		if (from_x.dot(to_z) > 0)
		{
			from_x = to_z;
		}
		else
		{
			from_x = -to_z;
		}
	}
	//y轴对齐
	dis_x = abs(from_y.dot(to_x));
	dis_y = abs(from_y.dot(to_y));
	dis_z = abs(from_y.dot(to_z));
	dis_max = dis_x;
	if (dis_y > dis_max)dis_max = dis_y;
	if (dis_z > dis_max)dis_max = dis_z;
	if (dis_x == dis_max)
	{
		if (from_y.dot(to_x) > 0)
		{
			from_y = to_x;
		}
		else
		{
			from_y = -to_x;
		}
	}
	if (dis_y == dis_max)
	{
		if (from_y.dot(to_y) > 0)
		{
			from_y = to_y;
		}
		else
		{
			from_y = -to_y;
		}
	}
	if (dis_z == dis_max)
	{
		if (from_y.dot(to_z) > 0)
		{
			from_y = to_z;
		}
		else
		{
			from_y = -to_z;
		}
	}
	//z轴对齐
	dis_x = abs(from_z.dot(to_x));
	dis_y = abs(from_z.dot(to_y));
	dis_z = abs(from_z.dot(to_z));
	dis_max = dis_x;
	if (dis_y > dis_max)dis_max = dis_y;
	if (dis_z > dis_max)dis_max = dis_z;
	if (dis_x == dis_max)
	{
		if (from_z.dot(to_x) > 0)
		{
			from_z = to_x;
		}
		else
		{
			from_z = -to_x;
		}
	}
	if (dis_y == dis_max)
	{
		if (from_z.dot(to_y) > 0)
		{
			from_z = to_y;
		}
		else
		{
			from_z = -to_y;
		}
	}
	if (dis_z == dis_max)
	{
		if (from_z.dot(to_z) > 0)
		{
			from_z = to_z;
		}
		else
		{
			from_z = -to_z;
		}
	}
	//
	/*m_from[0] = from_x[0];  m_from[4] = from_x[1];  m_from[8] = from_x[2];
	m_from[1] = from_y[0];	m_from[5] = from_y[1];  m_from[9] = from_y[2];
	m_from[2] = from_z[0];  m_from[6] = from_z[1];  m_from[10] = from_z[2];*/
	m_from[0] = from_x[0];  m_from[1] = from_x[1];  m_from[2] = from_x[2];
	m_from[4] = from_y[0];	m_from[5] = from_y[1];  m_from[6] = from_y[2];
	m_from[8] = from_z[0];  m_from[9] = from_z[1];  m_from[10] = from_z[2];
	activePart_form->m_frame->setFromMatrix(m_from);
}

void CLayoutRefiner::AllCoordinateAlignment(int index1, int index2)
{

	CShapePart* activePart_form = m_shapeHandler_h->m_allParts[index1];
	CShapePart* activePart_to = m_shapeHandler_h->m_allParts[index2];
	cv::Vec3d from_x, from_y, from_z, to_x, to_y, to_z;

	if (activePart_form == NULL)
	{
		return;
	}
	if (activePart_to == NULL)
	{
		return;
	}
	double m_from[16], m_to[16];
	activePart_form->m_frame->getMatrix(m_from);
	activePart_to->m_frame->getMatrix(m_to);
	from_x[0] = m_from[0]; from_x[1] = m_from[1]; from_x[2] = m_from[2];
	from_y[0] = m_from[4]; from_y[1] = m_from[5]; from_y[2] = m_from[6];
	from_z[0] = m_from[8]; from_z[1] = m_from[9]; from_z[2] = m_from[10];
	to_x[0] = m_to[0]; to_x[1] = m_to[1]; to_x[2] = m_to[2];
	to_y[0] = m_to[4]; to_y[1] = m_to[5]; to_y[2] = m_to[6];
	to_z[0] = m_to[8]; to_z[1] = m_to[9]; to_z[2] = m_to[10];
	double dis_max, dis_x, dis_y, dis_z;
	//x轴对齐
	dis_x = abs(from_x.dot(to_x));
	dis_y = abs(from_x.dot(to_y));
	dis_z = abs(from_x.dot(to_z));
	dis_max = dis_x;
	if (dis_y > dis_max)dis_max = dis_y;
	if (dis_z > dis_max)dis_max = dis_z;
	if (dis_x == dis_max)
	{
		if (from_x.dot(to_x)>0)
		{
			from_x = to_x;
		}
		else
		{
			from_x = -to_x;
		}
	}
	if (dis_y == dis_max)
	{
		if (from_x.dot(to_y) > 0)
		{
			from_x = to_y;
		}
		else
		{
			from_x = -to_y;
		}
	}
	if (dis_z == dis_max)
	{
		if (from_x.dot(to_z) > 0)
		{
			from_x = to_z;
		}
		else
		{
			from_x = -to_z;
		}
	}
	//y轴对齐
	dis_x = abs(from_y.dot(to_x));
	dis_y = abs(from_y.dot(to_y));
	dis_z = abs(from_y.dot(to_z));
	dis_max = dis_x;
	if (dis_y > dis_max)dis_max = dis_y;
	if (dis_z > dis_max)dis_max = dis_z;
	if (dis_x == dis_max)
	{
		if (from_y.dot(to_x) > 0)
		{
			from_y = to_x;
		}
		else
		{
			from_y = -to_x;
		}
	}
	if (dis_y == dis_max)
	{
		if (from_y.dot(to_y) > 0)
		{
			from_y = to_y;
		}
		else
		{
			from_y = -to_y;
		}
	}
	if (dis_z == dis_max)
	{
		if (from_y.dot(to_z) > 0)
		{
			from_y = to_z;
		}
		else
		{
			from_y = -to_z;
		}
	}
	//z轴对齐
	dis_x = abs(from_z.dot(to_x));
	dis_y = abs(from_z.dot(to_y));
	dis_z = abs(from_z.dot(to_z));
	dis_max = dis_x;
	if (dis_y > dis_max)dis_max = dis_y;
	if (dis_z > dis_max)dis_max = dis_z;
	if (dis_x == dis_max)
	{
		if (from_z.dot(to_x) > 0)
		{
			from_z = to_x;
		}
		else
		{
			from_z = -to_x;
		}
	}
	if (dis_y == dis_max)
	{
		if (from_z.dot(to_y) > 0)
		{
			from_z = to_y;
		}
		else
		{
			from_z = -to_y;
		}
	}
	if (dis_z == dis_max)
	{
		if (from_z.dot(to_z) > 0)
		{
			from_z = to_z;
		}
		else
		{
			from_z = -to_z;
		}
	}
	//
	/*m_from[0] = from_x[0];  m_from[4] = from_x[1];  m_from[8] = from_x[2];
	m_from[1] = from_y[0];	m_from[5] = from_y[1];  m_from[9] = from_y[2];
	m_from[2] = from_z[0];  m_from[6] = from_z[1];  m_from[10] = from_z[2];*/
	m_from[0] = from_x[0];  m_from[1] = from_x[1];  m_from[2] = from_x[2];
	m_from[4] = from_y[0];	m_from[5] = from_y[1];  m_from[6] = from_y[2];
	m_from[8] = from_z[0];  m_from[9] = from_z[1];  m_from[10] = from_z[2];
	activePart_form->m_frame->setFromMatrix(m_from);
}

cv::Vec3d CLayoutRefiner::Normalize(cv::Vec3d u)
{
	double d = sqrt(u[0] * u[0] + u[1] * u[1] + u[2] * u[2]);
	u[0] = u[0] / d;
	u[1] = u[1] / d;
	u[2] = u[2] / d;
	return u;
}

void CLayoutRefiner::ExecuteAlignmentOptimizing(std::vector<ShapeRelation> relations, qglviewer::Camera* c, std::vector<bool>* floatIndex)
{
	RotationOptimizing(&relations, c);
	//优化
	if (my_solver_ == NULL)
	{
		my_solver_ = new Solver();
	}
	vector<int> optimizing_cubes; //得到参与优化的shape编号
	for (int i = 0; i < relations.size(); i++)
	{
		ShapeRelation sr = relations[i];
		for (int j = 0; j < 2; j++)
		{
			int k = 0;
			for (k = 0; k < optimizing_cubes.size(); k++)
			{
				if (optimizing_cubes[k] == sr.edges[j].m_shapeIndex) break;
			}
			if (k == optimizing_cubes.size())
			{
				optimizing_cubes.push_back(sr.edges[j].m_shapeIndex);
			}

		}
	}
	shape_num_ = optimizing_cubes.size();
	Edge e = relations[0].edges[0];
	setmatrixM(e.m_3DTo2DMatrix);//3d投影到2d的矩阵
	for (int i = 0; i < optimizing_cubes.size(); i++)
	{
		m_shapeHandler_h->m_allParts[optimizing_cubes[i]]->m_optimizing_index = i;//设置优化shape的编号
		setShapeSize(m_shapeHandler_h->m_allParts[optimizing_cubes[i]]->m_shapeSize);//设置每个shape的大小
		setPosition2D(m_shapeHandler_h->m_allParts[optimizing_cubes[i]]->m_centerPointProjected);//设置每个shape的中心点的投影
	}
	//如果shape不是悬浮状态，则设置他们的位置
	vector<int> indexs;
	for (int i = 0; i < relations.size(); i++)
	{
		ShapeRelation sr = relations[i];
		for (int j = 0; j < 2; j++)
		{
			e = sr.edges[j];
			int k = 0;
			for (k = 0; k < indexs.size(); k++)
			{
				if (indexs[k] == m_shapeHandler_h->m_allParts[e.m_shapeIndex]->m_optimizing_index) break;
			}
			if (k == indexs.size())
			{

				indexs.push_back(m_shapeHandler_h->m_allParts[e.m_shapeIndex]->m_optimizing_index);
				if (!(*floatIndex)[e.m_shapeIndex])
				{
					setFirst3DPosition(m_shapeHandler_h->m_allParts[e.m_shapeIndex]->m_centerPoint, m_shapeHandler_h->m_allParts[e.m_shapeIndex]->m_optimizing_index);
				}
			}
		}
	}
	//用中心点参数化其他点
	for (int i = 0; i < relations.size(); i++)
	{
		ShapeRelation sr = relations[i];
		e = sr.edges[0];
		setConstraint_dir(e.m_dir3D);
		vector<vector<double>> references;
		//cout << " references is  ";
		for (int j = 0; j < 2; j++)
		{
			//layout_refiner.setReferenceMatrix()
			e = sr.edges[j];
			vector<double> vec;
			for (int k = 0; k < 9; k++)
			{
				vec.push_back(e.m_centerToVertex[k]);
			}
			vec.push_back(m_shapeHandler_h->m_allParts[e.m_shapeIndex]->m_optimizing_index);
			references.push_back(vec);
		}
		setReferenceMatrix(references);
	}

	cv::Mat upper_part_matrixA = getUpperPartMatrixA();
	std::vector<cv::Mat> mat_vector;
	computeConstraints2();
	for (int k = 0; k < constraints_mats_.size(); k++)
	{
		cv::Mat constraints_mat_ = constraints_mats_[k];
		mat_vector.push_back(constraints_mat_);
	}
	Solver::Sparse_matrix AT(upper_part_matrixA.rows + mat_vector.size() * 3, upper_part_matrixA.cols);
	Solver::Vector X(upper_part_matrixA.cols);
	Solver::Vector B(upper_part_matrixA.rows + mat_vector.size() * 3);
	//cout << "exe============================================\n";
	for (int i = 0; i < upper_part_matrixA.rows; i++)
	{
		for (int j = 0; j < upper_part_matrixA.cols; j++)
		{
			AT.add_coef(i, j, upper_part_matrixA.at<double>(i, j));
			//cout << "   " << upper_part_matrixA.at<double>(i, j);
		}
		//cout << "\n";
	}
	for (int m = 0; m < mat_vector.size(); m++)
	{
		cv::Mat constraints_mat_ = mat_vector[m];
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < constraints_mat_.cols; j++)
			{
				AT.add_coef(m * 3 + i + upper_part_matrixA.rows, j, 200000 * constraints_mat_.at<double>(i, j));
				//cout << "   " << 20000 * constraints_mat_.at<double>(i, j);
			}
			//cout << "\n";
		}
	}
	cv::Mat matB = getMatB();
	for (int i = 0; i < upper_part_matrixA.rows + mat_vector.size() * 3; i++)
	{
		B[i] = matB.at<double>(i, 0);
		//cout << B[i] << "\n";
	}
	if (my_solver_->precompute(AT))
	{
		my_solver_->solve(B, X);
	}
	else
	{
		cout << "matrix error" << "\n";
	}
	//CShapePart* activePart;
	float sx, sy, sz;
	int no, o_size;
	o_size = optimizing_cubes.size();
	for (int i = 0; i < optimizing_cubes.size(); i++)
	{

		CShapePart* activePart = m_shapeHandler_h->m_allParts[optimizing_cubes[i]];
		(*floatIndex)[optimizing_cubes[i]] = false;
		no = activePart->m_optimizing_index;
		cv::Vec3d shape_s;
		shape_s[0] = activePart->m_mesh->bbox.size()[0];
		shape_s[1] = activePart->m_mesh->bbox.size()[1];
		shape_s[2] = activePart->m_mesh->bbox.size()[2];
		sx = X[o_size * 3 + no * 3 + 0] / shape_s[0];
		sy = X[o_size * 3 + no * 3 + 1] / shape_s[1];
		sz = X[o_size * 3 + no * 3 + 2] / shape_s[2];
		activePart->m_frame->setPosition(qglviewer::Vec(X[no * 3 + 0], X[no * 3 + 1], X[no * 3 + 2]));
		trimesh::scale(activePart->m_mesh, sx, sy, sz);
		activePart->m_mesh->need_bbox();
		activePart->m_mesh->need_normals();
	}
}

double CLayoutRefiner::Compute3DDirAngle(cv::Vec3d cv_from, cv::Vec3d cv_to)
{
	const qreal epsilon = 1E-10;
	qglviewer::Vec from, to;
	from.x = cv_from[0]; from.y = cv_from[1]; from.z = cv_from[2];
	to.x = cv_to[0]; to.y = cv_to[1]; to.z = cv_to[2];
	const qreal fromSqNorm = from.squaredNorm();
	const qreal toSqNorm = to.squaredNorm();
	// Identity Quaternion when one vector is null
	if ((fromSqNorm < epsilon) || (toSqNorm < epsilon)) {
		qglviewer::Quaternion q;
		q[0] = q[1] = q[2] = 0.0;
		q[3] = 1.0;
		return 0;
	}
	else {
		qglviewer::Vec axis = cross(from, to);
		const qreal axisSqNorm = axis.squaredNorm();

		// Aligned vectors, pick any axis, not aligned with from or to
		if (axisSqNorm < epsilon)
			axis = from.orthogonalVec();

		qreal angle = asin(sqrt(axisSqNorm / (fromSqNorm * toSqNorm)));
		return angle;
	}
}

bool CLayoutRefiner::CheckErrorRelation(std::vector<ShapeRelation> relations, qglviewer::Camera* c, std::vector<bool> floatIndex)
{
	//优化
	if (my_solver_ == NULL)
	{
		my_solver_ = new Solver();
	}
	vector<int> optimizing_cubes;
	for (int i = 0; i < relations.size(); i++)
	{
		ShapeRelation sr = relations[i];
		for (int j = 0; j < 2; j++)
		{
			int k = 0;
			for (k = 0; k < optimizing_cubes.size(); k++)
			{
				if (optimizing_cubes[k] == sr.edges[j].m_shapeIndex) break;
			}
			if (k == optimizing_cubes.size())
			{
				optimizing_cubes.push_back(sr.edges[j].m_shapeIndex);
			}

		}
	}
	vector<int> indexs;
	int cube_size = 0;
	cube_size = optimizing_cubes.size();
	CLayoutRefiner layout_refiner(cube_size);
	Edge e = relations[0].edges[0];
	//matrix shape_size
	layout_refiner.setmatrixM(e.m_3DTo2DMatrix);
	for (int i = 0; i < optimizing_cubes.size(); i++)
	{
		m_shapeHandler_h->m_allParts[optimizing_cubes[i]]->m_optimizing_index = i;
		layout_refiner.setShapeSize(m_shapeHandler_h->m_allParts[optimizing_cubes[i]]->m_shapeSize);
		layout_refiner.setPosition2D(m_shapeHandler_h->m_allParts[optimizing_cubes[i]]->m_centerPointProjected);
	}

	for (int i = 0; i < relations.size(); i++)
	{
		ShapeRelation sr = relations[i];
		for (int j = 0; j < 2; j++)
		{
			e = sr.edges[j];
			int k = 0;
			for (k = 0; k < indexs.size(); k++)
			{
				if (indexs[k] == m_shapeHandler_h->m_allParts[e.m_shapeIndex]->m_optimizing_index) break;
			}
			if (k == indexs.size())
			{

				indexs.push_back(m_shapeHandler_h->m_allParts[e.m_shapeIndex]->m_optimizing_index);
				if (!floatIndex[e.m_shapeIndex])
				{
					layout_refiner.setFirst3DPosition(m_shapeHandler_h->m_allParts[e.m_shapeIndex]->m_centerPoint, m_shapeHandler_h->m_allParts[e.m_shapeIndex]->m_optimizing_index);
				}
			}
		}
	}
	for (int i = 0; i < relations.size(); i++)
	{
		ShapeRelation sr = relations[i];
		e = sr.edges[0];
		layout_refiner.setConstraint_dir(e.m_dir3D);
		vector<vector<double>> references;
		for (int j = 0; j < 2; j++)
		{
			//layout_refiner.setReferenceMatrix()
			e = sr.edges[j];
			vector<double> vec;
			for (int k = 0; k < 9; k++)
			{
				vec.push_back(e.m_centerToVertex[k]);
			}
			vec.push_back(m_shapeHandler_h->m_allParts[e.m_shapeIndex]->m_optimizing_index);
			references.push_back(vec);
		}
		layout_refiner.setReferenceMatrix(references);
	}
	cv::Mat upper_part_matrixA = layout_refiner.getUpperPartMatrixA();
	std::vector<cv::Mat> mat_vector;
	layout_refiner.computeConstraints2();
	for (int k = 0; k < layout_refiner.constraints_mats_.size(); k++)
	{
		cv::Mat constraints_mat_ = layout_refiner.constraints_mats_[k];
		mat_vector.push_back(constraints_mat_);
	}
	Solver::Sparse_matrix AT(upper_part_matrixA.rows + mat_vector.size() * 3, upper_part_matrixA.cols);
	Solver::Vector X(upper_part_matrixA.cols);
	Solver::Vector B(upper_part_matrixA.rows + mat_vector.size() * 3);
	//cout << "===================================================\n";
	for (int i = 0; i < upper_part_matrixA.rows; i++)
	{
		for (int j = 0; j < upper_part_matrixA.cols; j++)
		{
			AT.add_coef(i, j, upper_part_matrixA.at<double>(i, j));
			//cout << "   " << upper_part_matrixA.at<double>(i, j);
		}
		//cout << "\n";
	}
	for (int m = 0; m < mat_vector.size(); m++)
	{
		cv::Mat constraints_mat_ = mat_vector[m];
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < constraints_mat_.cols; j++)
			{
				AT.add_coef(m * 3 + i + upper_part_matrixA.rows, j, 200000 * constraints_mat_.at<double>(i, j));
				//cout << "   " << 20000 * constraints_mat_.at<double>(i, j);
			}
			//cout << "\n";
		}
	}
	cv::Mat matB = layout_refiner.getMatB();
	for (int i = 0; i < upper_part_matrixA.rows + mat_vector.size() * 3; i++)
	{
		B[i] = matB.at<double>(i, 0);
		//cout << B[i] << "\n";
	}
	//固定位置shape的size和位置约束加强，使得变化最小
	for (int i = 0; i < floatIndex.size();i++)
	{

	}
	if (my_solver_->precompute(AT))
	{
		my_solver_->solve(B, X);
	}
	else
	{
		std::cout << "matrix error" << std::endl;
	}

	//CShapePart* activePart;
	float sx, sy, sz;
	int no, o_size;
	o_size = optimizing_cubes.size();
	vector<cv::Vec3d> locations;
	vector<cv::Vec3d> scales;
	for (int i = 0; i < optimizing_cubes.size(); i++)
	{
		CShapePart* activePart = m_shapeHandler_h->m_allParts[optimizing_cubes[i]];
		no = activePart->m_optimizing_index;
		cv::Vec3d shape_s;
		shape_s[0] = activePart->m_mesh->bbox.size()[0];
		shape_s[1] = activePart->m_mesh->bbox.size()[1];
		shape_s[2] = activePart->m_mesh->bbox.size()[2];
		sx = X[o_size * 3 + no * 3 + 0] / shape_s[0];
		sy = X[o_size * 3 + no * 3 + 1] / shape_s[1];
		sz = X[o_size * 3 + no * 3 + 2] / shape_s[2];
		//size
		if (abs(sx) > 1.5 || abs(sy) > 1.5 || abs(sz) > 1.5 || abs(sx) < 0.7 || abs(sy) < 0.7 || abs(sz) < 0.7) //1.5 0.7
		{
			return false;
		}
		if (sx < 0 || sy < 0 || sz < 0)
		{
			return false;
		}
		cv::Vec3d location;
		location[0] = X[no * 3 + 0];
		location[1] = X[no * 3 + 1];
		location[2] = X[no * 3 + 2];
		locations.push_back(location);
		cv::Vec3d scale;
		scale[0] = X[o_size * 3 + no * 3 + 0];
		scale[1] = X[o_size * 3 + no * 3 + 1];
		scale[2] = X[o_size * 3 + no * 3 + 2];
		scales.push_back(scale);
	}
	for (int i = 0; i < locations.size(); i++)
	{
		for (int j = i+1; j < locations.size(); j++)
		{
			CShapePart activePart1 = *m_shapeHandler_h->m_allParts[optimizing_cubes[i]];
			CShapePart activePart2 = *m_shapeHandler_h->m_allParts[optimizing_cubes[j]];
			//两个frame移动到目标位置
			activePart1.m_frame->setPosition(qglviewer::Vec(locations[i][0], locations[i][1], locations[i][2]));
			activePart2.m_frame->setPosition(qglviewer::Vec(locations[j][0], locations[j][1], locations[j][2]));
			//
			cv::Vec3d  threshold = scales[i] / 2;
			cv::Vec3d minPoint = -scales[j]/2;
			cv::Vec3d maxPoint = scales[j]/2;
			//6个顶点
			std::vector<cv::Vec3d> points;
			cv::Vec3d p1, p2, p3, p4, p5, p6, p7, p8;
			p1 = p2 = p4 = p5 = minPoint;
			p7 = p6 = p8 = p3 = maxPoint;
			p2[1] = -p2[1];
			p4[0] = -p4[0];
			p5[2] = -p5[2];
			p6[0] = -p6[0];
			p8[1] = -p8[1];
			p3[2] = -p3[2];
			points.push_back(p1); points.push_back(p2); points.push_back(p3); points.push_back(p4);
			points.push_back(p5); points.push_back(p6); points.push_back(p7); points.push_back(p8);
			//local-world-local
			std::vector<cv::Vec3d> points1;
			for (int k = 0; k < points.size();k++)
			{
				qglviewer::Vec	globalPos = activePart2.m_frame->inverseCoordinatesOf(qglviewer::Vec(points[k][0], points[k][1], points[k][2]));
				qglviewer::Vec localPos = activePart1.m_frame->localCoordinatesOf(globalPos);
				points1.push_back(cv::Vec3d(localPos.x, localPos.y, localPos.z));

			}
			for (int k = 0; k < points1.size();k++)
			{
				if (threshold[0] + 0.05 >= abs(points1[k][0]) && threshold[1] + 0.05 >= abs(points1[k][1]) && threshold[2] + 0.05 >= abs(points1[k][2]))
				{
					/*	std::cout << "=====================CHECK\n";
						std::cout << "threshold is " << threshold << "\n";
						std::cout << "points1 is " << points1[k] << "\n";
						std::cout << "=====================CHECK\n";*/
					return false;
				}
			}
		}
	}
	return true;
}

bool CLayoutRefiner::CheckAndAddRelation(std::vector<ShapeRelation> relations, qglviewer::Camera* c, std::vector<bool> floatIndex, std::vector<ShapeRelation>* availablerelations)
{
	//克隆relations
	std::vector<ShapeRelation> clone_relations;
	for (int i = 0; i < relations.size(); i++)
	{
		clone_relations.push_back(relations[i]);
	}
	//旋转操作
	ShapeRelation sRelation;
	Edge e1, e2;
	int count = 0;
	for (count = 0; count < Utils::gdv_MaxIteNumbers; count++)
	{
		for (int j = 0; j < relations.size(); j++)
		{
			sRelation = relations[j];
			e1 = sRelation.edges[0];
			e2 = sRelation.edges[1];
			RotateWithOneRelaion(e1, e2);
			RotateWithOneRelaion(e2, e1);
			if (sRelation.is2relations)
			{
				RotateWithTwoRelaion(e1, e2);
				RotateWithTwoRelaion(e2, e1);
			}
			m_shapeHandler_h->ComputeEdgeAndNormal(c);
			UpdateVecRelations(relations);
		}
	}
	//输出旋转后的角度差
	/*for (int i = 0; i < relations.size(); i++)
	{
	ShapeRelation re = relations[i];
	cv::Vec3d dir_from, dir_to;
	dir_from = re.edges[0].m_dir3D;
	dir_to = re.edges[1].m_dir3D;
	cout << "relation before " << i << " is " << Compute3DDirAngle(dir_from, dir_to) << "\n";
	}*/

	//每两个shape只留下一条边
	for (int i = 0; i < relations.size(); i++)
	{
		ShapeRelation shapeRelation = relations[i];
		int shapeIndex1 = shapeRelation.edges[0].m_shapeIndex;
		int shapeIndex2 = shapeRelation.edges[1].m_shapeIndex;
		for (int j = i + 1; j < relations.size(); j++)
		{
			if (shapeIndex1 == relations[j].edges[0].m_shapeIndex&&shapeIndex2 == relations[j].edges[1].m_shapeIndex)
			{
				if (relations[j].is2relations)
					relations[i].is2relations = true;
				relations.erase(relations.begin() + j);
				j--;
			}
			if (shapeIndex1 == relations[j].edges[1].m_shapeIndex&&shapeIndex2 == relations[j].edges[0].m_shapeIndex)
			{
				if (relations[j].is2relations)
					relations[i].is2relations = true;
				relations.erase(relations.begin() + j);
				j--;
			}
		}

	}
	//深度优先搜索确定最终旋转
	vector<int> optimizing_cubes;
	for (int i = 0; i < relations.size(); i++)
	{
		ShapeRelation sr = relations[i];
		for (int j = 0; j < 2; j++)
		{
			int k = 0;
			for (k = 0; k < optimizing_cubes.size(); k++)
			{
				if (optimizing_cubes[k] == sr.edges[j].m_shapeIndex) break;
			}
			if (k == optimizing_cubes.size())
			{
				optimizing_cubes.push_back(sr.edges[j].m_shapeIndex);
			}

		}
	}

	int relationsSize = relations.size();
	for (int i = 0; i < m_shapeHandler_h->m_allParts.size(); i++)
	{
		m_shapeHandler_h->m_allParts[i]->m_isRotate = true;
	}
	vector<int> fixShapeNos;//记录固定的shape编号
	m_shapeHandler_h->m_allParts[relations[0].edges[0].m_shapeIndex]->m_isRotate = false;
	fixShapeNos.push_back(relations[0].edges[0].m_shapeIndex);
	while (true)
	{
		int fixShapeNOsSize_begin = fixShapeNos.size();
		for (int i = 0; i < fixShapeNos.size(); i++)
		{
			int fisShapeNo = fixShapeNos[i];
			for (int j = 0; j < relations.size(); j++)
			{
				int shapeNo[2];
				shapeNo[0] = relations[j].edges[0].m_shapeIndex;
				shapeNo[1] = relations[j].edges[1].m_shapeIndex;
				if (fisShapeNo == shapeNo[0])
				{
					if (m_shapeHandler_h->m_allParts[shapeNo[1]]->m_isRotate)
					{
						fixShapeNos.push_back(shapeNo[1]);
						m_shapeHandler_h->m_allParts[shapeNo[1]]->m_isRotate = false;
						if (relations[j].is2relations)
							AllCoordinateAlignment(relations[j].edges[1], relations[j].edges[0]);
						else
							OneCoordinateAlignment(relations[j].edges[1], relations[j].edges[0]);
					}
				}
				else if (fisShapeNo == shapeNo[1])
				{
					if (m_shapeHandler_h->m_allParts[shapeNo[0]]->m_isRotate)
					{
						fixShapeNos.push_back(shapeNo[0]);
						m_shapeHandler_h->m_allParts[shapeNo[0]]->m_isRotate = false;
						if (relations[j].is2relations)
							AllCoordinateAlignment(relations[j].edges[0], relations[j].edges[1]);
						else
							OneCoordinateAlignment(relations[j].edges[0], relations[j].edges[1]);
					}
				}
				m_shapeHandler_h->ComputeEdgeAndNormal(c);
				UpdateVecRelations(relations);
			}
		}
		if (optimizing_cubes.size() == fixShapeNos.size())
		{
			break;
		}
		int fixShapeNOsSize_end = fixShapeNos.size();
		if (fixShapeNOsSize_begin == fixShapeNOsSize_end)
		{
			for (int i = 0; i < relations.size(); i++)
			{
				int shapeNo[2];
				shapeNo[0] = relations[i].edges[0].m_shapeIndex;
				shapeNo[1] = relations[i].edges[1].m_shapeIndex;

				if (m_shapeHandler_h->m_allParts[shapeNo[0]]->m_isRotate)
				{
					m_shapeHandler_h->m_allParts[shapeNo[0]]->m_isRotate = false;
					fixShapeNos.push_back(shapeNo[0]);
					break;
				}
				else if (m_shapeHandler_h->m_allParts[shapeNo[1]]->m_isRotate)
				{
					m_shapeHandler_h->m_allParts[shapeNo[1]]->m_isRotate = false;
					fixShapeNos.push_back(shapeNo[1]);
					break;
				}
			}

		}
	}
	//cout << "--------------------------------------\n";
	////输出旋转后的角度差
	//for (int i = 0; i < relations.size(); i++)
	//{
	//	ShapeRelation re = relations[i];
	//	cv::Vec3d dir_from, dir_to;
	//	dir_from = re.edges[0].m_dir3D;
	//	dir_to = re.edges[1].m_dir3D;
	//	cout << "relation before " << i << " is " << Compute3DDirAngle(dir_from, dir_to) << "\n";
	//}


	//优化
	if (my_solver_ == NULL)
	{
		my_solver_ = new Solver();
	}
	relations.clear();
	for (int i = 0; i < clone_relations.size(); i++)
	{
		relations.push_back(clone_relations[i]);
	}
	UpdateVecRelations(relations);
	optimizing_cubes.clear();
	for (int i = 0; i < relations.size(); i++)
	{
		ShapeRelation sr = relations[i];
		for (int j = 0; j < 2; j++)
		{
			int k = 0;
			for (k = 0; k < optimizing_cubes.size(); k++)
			{
				if (optimizing_cubes[k] == sr.edges[j].m_shapeIndex) break;
			}
			if (k == optimizing_cubes.size())
			{
				optimizing_cubes.push_back(sr.edges[j].m_shapeIndex);
			}

		}
	}
	vector<int> indexs;
	int cube_size = 0;
	cube_size = optimizing_cubes.size();
	CLayoutRefiner layout_refiner(cube_size);
	Edge e = relations[0].edges[0];
	//matrix shape_size
	layout_refiner.setmatrixM(e.m_3DTo2DMatrix);
	for (int i = 0; i < optimizing_cubes.size(); i++)
	{
		m_shapeHandler_h->m_allParts[optimizing_cubes[i]]->m_optimizing_index = i;
		layout_refiner.setShapeSize(m_shapeHandler_h->m_allParts[optimizing_cubes[i]]->m_shapeSize);
		layout_refiner.setPosition2D(m_shapeHandler_h->m_allParts[optimizing_cubes[i]]->m_centerPointProjected);
	}

	for (int i = 0; i < relations.size(); i++)
	{
		ShapeRelation sr = relations[i];
		for (int j = 0; j < 2; j++)
		{
			e = sr.edges[j];
			int k = 0;
			for (k = 0; k < indexs.size(); k++)
			{
				if (indexs[k] == m_shapeHandler_h->m_allParts[e.m_shapeIndex]->m_optimizing_index) break;
			}
			if (k == indexs.size())
			{

				indexs.push_back(m_shapeHandler_h->m_allParts[e.m_shapeIndex]->m_optimizing_index);
				if (!floatIndex[e.m_shapeIndex])
				{
					layout_refiner.setFirst3DPosition(m_shapeHandler_h->m_allParts[e.m_shapeIndex]->m_centerPoint, m_shapeHandler_h->m_allParts[e.m_shapeIndex]->m_optimizing_index);
				}
			}
		}
	}
	for (int i = 0; i < relations.size(); i++)
	{
		ShapeRelation sr = relations[i];
		e = sr.edges[0];
		layout_refiner.setConstraint_dir(e.m_dir3D);
		vector<vector<double>> references;
		//cout << " references is  ";
		for (int j = 0; j < 2; j++)
		{
			//layout_refiner.setReferenceMatrix()
			e = sr.edges[j];
			vector<double> vec;
			for (int k = 0; k < 9; k++)
			{
				vec.push_back(e.m_centerToVertex[k]);
			}
			vec.push_back(m_shapeHandler_h->m_allParts[e.m_shapeIndex]->m_optimizing_index);
			references.push_back(vec);
		}
		layout_refiner.setReferenceMatrix(references);
	}
	cv::Mat upper_part_matrixA = layout_refiner.getUpperPartMatrixA();
	std::vector<cv::Mat> mat_vector;
	layout_refiner.computeConstraints2();
	for (int k = 0; k < layout_refiner.constraints_mats_.size(); k++)
	{
		cv::Mat constraints_mat_ = layout_refiner.constraints_mats_[k];
		mat_vector.push_back(constraints_mat_);
	}
	Solver::Sparse_matrix AT(upper_part_matrixA.rows, upper_part_matrixA.cols);
	Solver::Vector X(upper_part_matrixA.cols);
	Solver::Vector B(upper_part_matrixA.rows);
	for (int i = 0; i < upper_part_matrixA.rows; i++)
	{
		for (int j = 0; j < upper_part_matrixA.cols; j++)
		{
			AT.add_coef(i, j, upper_part_matrixA.at<double>(i, j));
			cout << "   " << upper_part_matrixA.at<double>(i, j);
		}
		cout << "\n";
	}
	cv::Mat matB = layout_refiner.getMatB();
	for (int i = 0; i < upper_part_matrixA.rows; i++)
	{
		B[i] = matB.at<double>(i, 0);
		//std::cout << " " << B[i] << "\n";
	}
	//添加constraints
	cv::Mat AT_append = cv::Mat::zeros(mat_vector.size() * 3, upper_part_matrixA.cols, CV_64F);
	for (int m = 0; m < mat_vector.size(); m++)
	{
		cv::Mat constraints_mat_ = mat_vector[m];
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < constraints_mat_.cols; j++)
			{
				AT_append.at<double>(m * 3 + i, j) = 20000 * constraints_mat_.at<double>(i, j);
			}
		}
	}
	/*my_solver_->precompute(AT);
	for (int i = 0; i < mat_vector.size(); i++)
	{
	Solver::Sparse_matrix matC(3, upper_part_matrixA.cols);
	for (int j = 0; j < upper_part_matrixA.cols; j++)
	{
	matC.add_coef(0, j, 20000 * AT_append.at<double>(i*3, j));
	matC.add_coef(1, j, 20000 * AT_append.at<double>(i*3+1, j));
	matC.add_coef(2, j, 20000 * AT_append.at<double>(i*3+2, j));
	}
	Solver::Vector optB=SetAddConstrisB(B);
	my_solver_->update_factorization(matC);

	}*/
	if (my_solver_->precompute(AT))
	{
		my_solver_->solve(B, X);
	}
	else
	{
		cout << "matrix error" << "\n";
	}


	//CShapePart* activePart;
	float sx, sy, sz;
	int no, o_size;
	o_size = optimizing_cubes.size();
	for (int i = 0; i < optimizing_cubes.size(); i++)
	{

		CShapePart* activePart = m_shapeHandler_h->m_allParts[optimizing_cubes[i]];
		floatIndex[optimizing_cubes[i]] = false;
		no = activePart->m_optimizing_index;
		cv::Vec3d shape_s;
		shape_s[0] = activePart->m_mesh->bbox.size()[0];
		shape_s[1] = activePart->m_mesh->bbox.size()[1];
		shape_s[2] = activePart->m_mesh->bbox.size()[2];
		sx = X[o_size * 3 + no * 3 + 0] / shape_s[0];
		sy = X[o_size * 3 + no * 3 + 1] / shape_s[1];
		sz = X[o_size * 3 + no * 3 + 2] / shape_s[2];
		if (abs(sx) > 1.5 || abs(sy) > 1.5 || abs(sz) > 1.5 || abs(sx) < 0.7 || abs(sy) < 0.7 || abs(sz) < 0.7)
		{
			return false;
		}

	}
	return true;
}

Solver::Vector CLayoutRefiner::SetAddConstrisB(Solver::Vector B)
{

	Solver::Vector optB(B.size() + 3);
	for (int i = 0; i < B.size(); i++)
	{
		optB[i] = B[i];
	}
	optB[B.size()] = 0;
	optB[B.size() + 1] = 0;
	optB[B.size() + 2] = 0;
	return optB;
}

Solver::Vector CLayoutRefiner::SetDeleteConstrisB(Solver::Vector B)
{
	Solver::Vector optB(B.size() - 3);
	for (int i = 0; i < B.size() - 3; i++)
	{
		optB[i] = B[i];
	}
	return optB;
}

void CLayoutRefiner::RotationOptimizing(std::vector<ShapeRelation>* relations, qglviewer::Camera* c)
{
	//克隆relations
	std::vector<ShapeRelation> clone_relations;
	for (int i = 0; i < (*relations).size(); i++)
	{
		clone_relations.push_back((*relations)[i]);
	}
	//旋转操作
	ShapeRelation sRelation;
	Edge e1, e2;
	int count = 0;
	int elapsed;
	for (count = 0; count < Utils::gdv_MaxIteNumbers; count++)
	{
		for (int j = 0; j < (*relations).size(); j++)
		{
			sRelation = (*relations)[j];
			e1 = sRelation.edges[0];
			e2 = sRelation.edges[1];
			RotateWithOneRelaion(e1, e2);
			RotateWithOneRelaion(e2, e1);
			if (sRelation.is2relations)
			{
				RotateWithTwoRelaion(e1, e2);
				RotateWithTwoRelaion(e2, e1);
			}
			//更新边和关系
			m_shapeHandler_h->ComputeEdgeAndNormal(c, e1);
			m_shapeHandler_h->ComputeEdgeAndNormal(c, e2);
			UpdateVecRelations((*relations));//long time
		}
	}
	//每两个shape只留下一条边
	for (int i = 0; i < (*relations).size(); i++)
	{
		ShapeRelation shapeRelation = (*relations)[i];
		int shapeIndex1 = shapeRelation.edges[0].m_shapeIndex;
		int shapeIndex2 = shapeRelation.edges[1].m_shapeIndex;
		for (int j = i + 1; j < (*relations).size(); j++)
		{
			if (shapeIndex1 == (*relations)[j].edges[0].m_shapeIndex&&shapeIndex2 == (*relations)[j].edges[1].m_shapeIndex)
			{
				if ((*relations)[j].is2relations)
					(*relations)[i].is2relations = true;
				(*relations).erase((*relations).begin() + j);
				j--;
			}
			if (shapeIndex1 == (*relations)[j].edges[1].m_shapeIndex&&shapeIndex2 == (*relations)[j].edges[0].m_shapeIndex)
			{
				if ((*relations)[j].is2relations)
					(*relations)[i].is2relations = true;
				(*relations).erase((*relations).begin() + j);
				j--;
			}
		}

	}
	//std::cout << "=================Graphmtx start\n";
	//构建图并检测出图中的环
	Graphmtx g(m_shapeHandler_h->m_allParts.size());
	//构造图
	//输入顶点
	for (int i = 0; i < m_shapeHandler_h->m_allParts.size(); i++)
	{
		g.insertVertex(i);
	}
	ShapeRelation **loopsRelation;
	loopsRelation = new ShapeRelation *[m_shapeHandler_h->m_allParts.size()];
	for (int i = 0; i < m_shapeHandler_h->m_allParts.size(); i++)
		loopsRelation[i] = new ShapeRelation[m_shapeHandler_h->m_allParts.size()];
	for (int i = 0; i < relations->size(); i++)
	{
		ShapeRelation relation = (*relations)[i];
		int e1 = relation.edges[0].m_shapeIndex;
		int e2 = relation.edges[1].m_shapeIndex;
		int j = g.getVertexPos(e1);//查顶点号  
		int k = g.getVertexPos(e2);
		loopsRelation[j][k] = relation;
		ShapeRelation relation2 = relation;
		relation2.edges[0] = relation.edges[1];
		relation2.edges[1] = relation.edges[0];
		loopsRelation[k][j] = relation2;
		g.insertEdge(j, k, 1);
	}
	//g.outputGraph();    //显示图  
	//std::cout << "=================outputGraph no error\n";
	//检测并输出环
	std::set<int> recordLoopVertex;  //已经在环上的点不能在作为环的起点
	std::vector<std::vector<int>> loops;  //保存多个环
	for (g.heap = 0; g.heap < g.maxVertices; g.heap++)
	{
		if (recordLoopVertex.count(g.heap)>0)
			continue;
		for (int i = 0; i < g.maxVertices; i++)
		{
			g.visitedFlag[i] = 0;
		}
		if (g.visitedFlag[g.heap] == 0)
		{
			//printf("\n-------------------the loop start and end with %d----------------\n", g.heap);
			g.clear_stack(&g.loop_stack);
			g.innerStep = 0;
			g.isRecall = 0;
			g.recordLoopVertex.clear();
			/*std::cout << "=================g.heap is " << g.heap<<"\n";
			std::cout << "=================g.maxVertices is " << g.maxVertices << "\n";
			std::cout << "=================recordLoopVertex size  is " << recordLoopVertex.size()<< "\n";
			std::cout << "=================loops size  is " << loops.size() << "\n";
			std::cout << "=================DFS start\n";*/
			g.DFS(g.heap);
			//std::cout << "=================DFS end\n";
			std::vector<int> loop;

			for (set<int>::iterator it = g.recordLoopVertex.begin(); it != g.recordLoopVertex.end(); ++it)
			{
				recordLoopVertex.insert(*it);
				loop.push_back(*it);
			}
			if (loop.size()>0)
				loops.push_back(loop);
		}
	}
	//std::cout << "=================Graphmtx end\n";
	//输出环
	/*for (int i = 0; i < loops.size(); i++)
	{
	std::cout << "loop is ";
	for (int j = 0; j < loops[i].size(); j++)
	{
	std::cout << loops[i][j] << " ";
	}
	std::cout << "\n";
	}*/
	set<int> optimizing_cubes;	//记录优化的所有shape的索引
	for (int i = 0; i < (*relations).size(); i++)
	{
		ShapeRelation sr = (*relations)[i];
		optimizing_cubes.insert(sr.edges[0].m_shapeIndex);
		optimizing_cubes.insert(sr.edges[1].m_shapeIndex);
	}
	int relationsSize = (*relations).size();
	for (int i = 0; i < m_shapeHandler_h->m_allParts.size(); i++)
	{
		m_shapeHandler_h->m_allParts[i]->m_isRotate = true;	//所有的shape都可以旋转
	}
	//vector<int> fixShapeNos;//记录固定的shape编号
	std::set<int> fixShapeNos;
	//先旋转环
	for (int i = 0; i < loops.size(); i++)
	{
		std::vector<int> temp_loop = loops[i];
		if (temp_loop.size() == 0)
			continue;
		int flag = 0;
		int start;
		for (int j = 0; j < temp_loop.size(); j++)
		{
			if (fixShapeNos.count(temp_loop[j])>0)
			{
				start = temp_loop[j];
				flag = 1;
				break;
			}
		}
		if (flag == 0)
		{
			start = temp_loop[0];
			fixShapeNos.insert(start);
			m_shapeHandler_h->m_allParts[start]->m_isRotate = false;
		}
		for (int j = 0; j < temp_loop.size(); j++)
		{
			AllCoordinateAlignment(temp_loop[j], start);
			m_shapeHandler_h->ComputeEdgeAndNormal(c, temp_loop[j]);
			fixShapeNos.insert(temp_loop[j]);
			m_shapeHandler_h->m_allParts[temp_loop[j]]->m_isRotate = false;
		}
	}

	if (fixShapeNos.size() == 0)
	{
		m_shapeHandler_h->m_allParts[(*relations)[0].edges[0].m_shapeIndex]->m_isRotate = false;
		fixShapeNos.insert((*relations)[0].edges[0].m_shapeIndex);
	}
	//深度优先搜索确定最终旋转
	while (true)
	{
		int fixShapeNos_startSize = fixShapeNos.size();
		for (int i = 0; i < (*relations).size(); i++)
		{
			ShapeRelation temp_r = (*relations)[i];
			int shapeIndex1 = temp_r.edges[0].m_shapeIndex;
			int shapeIndex2 = temp_r.edges[1].m_shapeIndex;
			if (fixShapeNos.count(shapeIndex1)>0 && fixShapeNos.count(shapeIndex2) == 0)
			{
				if (m_shapeHandler_h->m_allParts[shapeIndex2]->m_isRotate)
				{
					fixShapeNos.insert(shapeIndex2);
					m_shapeHandler_h->m_allParts[shapeIndex2]->m_isRotate = false;
					if (temp_r.is2relations)
						AllCoordinateAlignment(temp_r.edges[1], temp_r.edges[0]);
					else
						OneCoordinateAlignment(temp_r.edges[1], temp_r.edges[0]);
				}
				m_shapeHandler_h->ComputeEdgeAndNormal(c, shapeIndex2);
			}
			else if (fixShapeNos.count(shapeIndex1) == 0 && fixShapeNos.count(shapeIndex2)>0)
			{
				if (m_shapeHandler_h->m_allParts[shapeIndex1]->m_isRotate)
				{
					fixShapeNos.insert(shapeIndex1);
					m_shapeHandler_h->m_allParts[shapeIndex1]->m_isRotate = false;
					if (temp_r.is2relations)
						AllCoordinateAlignment(temp_r.edges[0], temp_r.edges[1]);
					else
						OneCoordinateAlignment(temp_r.edges[0], temp_r.edges[1]);
				}
				m_shapeHandler_h->ComputeEdgeAndNormal(c, shapeIndex1);
			}
		}
		if (fixShapeNos.size() == optimizing_cubes.size())
		{
			break;
		}
		if (fixShapeNos.size() == fixShapeNos_startSize)
		{
			for (int i = 0; i < (*relations).size(); i++)
			{
				ShapeRelation temp_r = (*relations)[i];
				int shapeIndex1 = temp_r.edges[0].m_shapeIndex;
				int shapeIndex2 = temp_r.edges[1].m_shapeIndex;
				if (fixShapeNos.count(shapeIndex1) == 0 && fixShapeNos.count(shapeIndex2) == 0)
				{
					m_shapeHandler_h->m_allParts[shapeIndex1]->m_isRotate = false;
					fixShapeNos.insert(shapeIndex1);
					break;
				}
			}
		}
	}
	(*relations).clear();
	for (int i = 0; i < clone_relations.size(); i++)
	{
		(*relations).push_back(clone_relations[i]);
	}
	UpdateVecRelations((*relations));
}

void CLayoutRefiner::ExecuteSpacingOptimizing(std::vector<ShapeRelation> relations, qglviewer::Camera* c, std::vector<std::vector<RecordNo>> recordNoss)
{
	//优化
	if (my_solver_ == NULL)
	{
		my_solver_ = new Solver();
	}
	
	vector<int> optimizing_cubes; //得到参与优化的shape编号
	for (int i = 0; i < relations.size(); i++)
	{
		ShapeRelation sr = relations[i];
		for (int j = 0; j < 2; j++)
		{
			int k = 0;
			for (k = 0; k < optimizing_cubes.size(); k++)
			{
				if (optimizing_cubes[k] == sr.edges[j].m_shapeIndex) break;
			}
			if (k == optimizing_cubes.size())
			{
				optimizing_cubes.push_back(sr.edges[j].m_shapeIndex);
			}

		}
	}
	shape_num_ = optimizing_cubes.size();
	Edge e = relations[0].edges[0];
	setmatrixM(e.m_3DTo2DMatrix);//3d投影到2d的矩阵
	for (int i = 0; i < optimizing_cubes.size(); i++)
	{
		m_shapeHandler_h->m_allParts[optimizing_cubes[i]]->m_optimizing_index = i;//设置优化shape的编号
		setShapeSize(m_shapeHandler_h->m_allParts[optimizing_cubes[i]]->m_shapeSize);//设置每个shape的大小
		setPosition2D(m_shapeHandler_h->m_allParts[optimizing_cubes[i]]->m_centerPointProjected);//设置每个shape的中心点的投影
	}
	//如果shape不是悬浮状态，则设置他们的位置
	vector<int> indexs;
	for (int i = 0; i < relations.size(); i++)
	{
		ShapeRelation sr = relations[i];
		for (int j = 0; j < 2; j++)
		{
			e = sr.edges[j];
			int k = 0;
			for (k = 0; k < indexs.size(); k++)
			{
				if (indexs[k] == m_shapeHandler_h->m_allParts[e.m_shapeIndex]->m_optimizing_index) break;
			}
			if (k == indexs.size())
			{
				indexs.push_back(m_shapeHandler_h->m_allParts[e.m_shapeIndex]->m_optimizing_index);
				setFirst3DPosition(m_shapeHandler_h->m_allParts[e.m_shapeIndex]->m_centerPoint, m_shapeHandler_h->m_allParts[e.m_shapeIndex]->m_optimizing_index);
			}
		}
	}
	//用中心点参数化其他点
	for (int i = 0; i < relations.size(); i++)
	{
		ShapeRelation sr = relations[i];
		e = sr.edges[0];
		setConstraint_dir(e.m_dir3D);
		vector<vector<double>> references;
		//cout << " references is  ";
		for (int j = 0; j < 2; j++)
		{
			//layout_refiner.setReferenceMatrix()
			e = sr.edges[j];
			vector<double> vec;
			for (int k = 0; k < 9; k++)
			{
				vec.push_back(e.m_centerToVertex[k]);
			}
			vec.push_back(m_shapeHandler_h->m_allParts[e.m_shapeIndex]->m_optimizing_index);
			references.push_back(vec);
		}
		setReferenceMatrix(references);
	}
	///spacing约束
	for (int i = 0; i < recordNoss.size(); i++)
	{
		vector<vector<vector<double>>> spacingReferences;
		for (int j = 0; j < recordNoss[i].size(); j++)
		{
			vector<vector<double>> references;
			RecordNo no = recordNoss[i][j];
			cv::Vec2d point1, point2;
			point1[0] = (m_shapeHandler_h->m_allParts[no.shape_no1]->edges[no.edge_no1].m_projectPt2D[no.vertices_no1][0]);
			point1[1] = (m_shapeHandler_h->m_allParts[no.shape_no1]->edges[no.edge_no1].m_projectPt2D[no.vertices_no1][1]);
			point2[0] = (m_shapeHandler_h->m_allParts[no.shape_no2]->edges[no.edge_no2].m_projectPt2D[no.vertices_no2][0]);
			point2[1] = (m_shapeHandler_h->m_allParts[no.shape_no2]->edges[no.edge_no2].m_projectPt2D[no.vertices_no2][1]);
			//layout_refiner.setReferenceMatrix()
			//point1
			e = m_shapeHandler_h->m_allParts[no.shape_no1]->edges[no.edge_no1];
			vector<double> vec;
			if (Utils::Calculate2DTwoPointDistance(point1, e.m_projectPt2D[0]) < 0.1)
			{
				for (int k = 0; k < 9; k++)
				{
					vec.push_back(e.m_centerToVertex[k]);
				}
			}
			else if (Utils::Calculate2DTwoPointDistance(point1, e.m_projectPt2D[1]) < 0.1)
			{
				for (int k = 0; k < 9; k++)
				{
					vec.push_back(e.m_centerToVertex1[k]);
				}
			}
			vec.push_back(m_shapeHandler_h->m_allParts[no.shape_no1]->m_optimizing_index);
			references.push_back(vec);
			//point2
			e = m_shapeHandler_h->m_allParts[no.shape_no2]->edges[no.edge_no2];
			vec.clear();
			if (Utils::Calculate2DTwoPointDistance(point2, e.m_projectPt2D[0]) < 0.1)
			{
				for (int k = 0; k < 9; k++)
				{
					vec.push_back(e.m_centerToVertex[k]);
				}
			}
			else if (Utils::Calculate2DTwoPointDistance(point2, e.m_projectPt2D[1]) < 0.1)
			{
				for (int k = 0; k < 9; k++)
				{
					vec.push_back(e.m_centerToVertex1[k]);
				}
			}
			vec.push_back(m_shapeHandler_h->m_allParts[no.shape_no2]->m_optimizing_index);
			references.push_back(vec);
			spacingReferences.push_back(references);
			//////////////////////////////////////////////////////////////////////////
			/*cv::Vec2d dir = point1 - point2;
			std::cout << "recordNoss dir is " << dir << "\n";*/
		}
		setSpaingMatrix(spacingReferences);
	}
	//
	cv::Mat upper_part_matrixA = getSpacingUpperPartMatrixA();
	std::vector<cv::Mat> mat_vector;
	computeConstraints2();
	computeSpacingConstraints();
	
	for (int k = 0; k < constraints_mats_.size(); k++)
	{
		cv::Mat constraints_mat_ = constraints_mats_[k];
		mat_vector.push_back(constraints_mat_);
	}
	std::vector<cv::Mat> spacing_mat_vector;
	for (int k = 0; k < spacingConstraints_mats_.size();k++)
	{
		cv::Mat constraints_mat_ = spacingConstraints_mats_[k];
		spacing_mat_vector.push_back(constraints_mat_);
	}
//	std::cout << "==========================spacing_mat_vector is " << spacing_mat_vector.size() << "\n";
	Solver::Sparse_matrix AT(upper_part_matrixA.rows + mat_vector.size() * 3+spacing_mat_vector.size(), upper_part_matrixA.cols);
	Solver::Vector X(upper_part_matrixA.cols);
	Solver::Vector B(upper_part_matrixA.rows + mat_vector.size() * 3 + spacing_mat_vector.size());
	//cout << "exe============================================\n";
	for (int i = 0; i < upper_part_matrixA.rows; i++)
	{
		for (int j = 0; j < upper_part_matrixA.cols; j++)
		{
			/*if (i<shape_num_*3)
			{
			AT.add_coef(i, j, upper_part_matrixA.at<double>(i, j)*0);
			cout << "   " << upper_part_matrixA.at<double>(i, j) * 0;
			}
			else
			{
			AT.add_coef(i, j, upper_part_matrixA.at<double>(i, j));
			cout << "   " << upper_part_matrixA.at<double>(i, j);
			}*/
			AT.add_coef(i, j, upper_part_matrixA.at<double>(i, j));
			//cout << "   " << upper_part_matrixA.at<double>(i, j);
			
		}
		//cout << "\n";
	}
	for (int m = 0; m < mat_vector.size(); m++)
	{
		cv::Mat constraints_mat_ = mat_vector[m];
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < constraints_mat_.cols; j++)
			{
				AT.add_coef(m * 3 + i + upper_part_matrixA.rows, j, 5000*constraints_mat_.at<double>(i, j));
				//cout << "   " << 50 * constraints_mat_.at<double>(i, j);
			}
			//cout << "\n";
		}
	}
	for (int n = 0; n < spacing_mat_vector.size();n++)
	{
		cv::Mat constraints_mat_ = spacing_mat_vector[n];
		for (int j = 0; j < constraints_mat_.cols; j++)
		{
			AT.add_coef(n + mat_vector.size() + upper_part_matrixA.rows, j, 5000 * constraints_mat_.at<double>(0, j));
			//cout << "   " << constraints_mat_.at<double>(0, j);
		}
		//cout << "\n";
	}
	cv::Mat matB = getSpacingMatB();
	for (int i = 0; i < upper_part_matrixA.rows + mat_vector.size() * 3 + spacing_mat_vector.size(); i++)
	{
		B[i] = matB.at<double>(i, 0);
		//cout << B[i] << "\n";
	}
	if (my_solver_->precompute(AT))
	{
		my_solver_->solve(B, X);
	}
	else
	{
		//cout << "matrix error" << "\n";
	}
	//
	//for (int i = 0; i < recordNoss.size(); i++)
	//{
	//	std::vector<RecordNo> recordNos = recordNoss[i];
	//	for (int p = 0; p < recordNos.size(); p++)
	//	{
	//		RecordNo no = recordNos[p];
	//		std::cout << "shape1  " << no.shape_no1 << "  shape2  " << no.shape_no2 << "  \n";
	//		cv::Vec2d point1, point2;
	//		point1[0] = (m_shapeHandler_h->m_allParts[no.shape_no1]->edges[no.edge_no1].m_projectPt2D[no.vertices_no1][0]);
	//		point1[1] = (m_shapeHandler_h->m_allParts[no.shape_no1]->edges[no.edge_no1].m_projectPt2D[no.vertices_no1][1]);
	//		point2[0] = (m_shapeHandler_h->m_allParts[no.shape_no2]->edges[no.edge_no2].m_projectPt2D[no.vertices_no2][0]);
	//		point2[1] = (m_shapeHandler_h->m_allParts[no.shape_no2]->edges[no.edge_no2].m_projectPt2D[no.vertices_no2][1]);
	//		cv::Vec2d dir = point1 - point2;
	//		std::cout << "recordNoss dir is " << dir << "\n";
	//	}
	//}
	//CShapePart* activePart;
	float sx, sy, sz;
	int no, o_size;
	o_size = optimizing_cubes.size();
	for (int i = 0; i < optimizing_cubes.size(); i++)
	{
		CShapePart* activePart = m_shapeHandler_h->m_allParts[optimizing_cubes[i]];
		no = activePart->m_optimizing_index;
		cv::Vec3d shape_s;
		shape_s[0] = activePart->m_mesh->bbox.size()[0];
		shape_s[1] = activePart->m_mesh->bbox.size()[1];
		shape_s[2] = activePart->m_mesh->bbox.size()[2];
		sx = X[o_size * 3 + no * 3 + 0] / shape_s[0];
		sy = X[o_size * 3 + no * 3 + 1] / shape_s[1];
		sz = X[o_size * 3 + no * 3 + 2] / shape_s[2];
		activePart->m_frame->setPosition(qglviewer::Vec(X[no * 3 + 0], X[no * 3 + 1], X[no * 3 + 2]));
		trimesh::scale(activePart->m_mesh, sx, sy, sz);
		activePart->m_mesh->need_bbox();
		activePart->m_mesh->need_normals();
	}
	//////////////////////////////////////////////////////////////////////////
	
}

void CLayoutRefiner::setSpaingMatrix(vector<vector<vector<double>>> matrix)
{
	spacing_Matrix_vector.push_back(matrix);
}

void CLayoutRefiner::computeSpacingConstraints()
{
	//std::cout << "spacing_Matrix_vector size is " << spacing_Matrix_vector.size()<<"\n";
	if (spacing_Matrix_vector.empty())
		return;
	int no1 = -1;
	int no2 = -1;
	int no3 = -1;
	int no4 = -1;
	for (int i = 0; i < spacing_Matrix_vector.size(); i++)
	{
		//一个向量组，组内两两相等
		vector<vector<vector<double>>> spacingReferences=spacing_Matrix_vector[i];
		if (spacingReferences.size() < 2)
			return;
		for (int j = 0; j < spacingReferences.size()-1;j++)
		{
			vector<vector<double>> references = spacingReferences[j];
			//第一个向量m2-m1
			double m1[9],m2[9];
			for (int k = 0; k < 9; k++)
			{
				m1[k] = references[0][k];
			}
			no1 = references[0][9];
			for (int k = 0; k < 9; k++)
			{
				m2[k] = references[1][k];
			}
			no2 = references[1][9];
			//第一个向量m4-m3
			double m3[9], m4[9];
			references = spacingReferences[j + 1];
			for (int k = 0; k < 9; k++)
			{
				m3[k] = references[0][k];
			}
			no3 = references[0][9];
			for (int k = 0; k < 9; k++)
			{
				m4[k] = references[1][k];
			}
			no4 = references[1][9];
			if (no2!=no3)
			{
				double temp1[9], temp2[9], temp3[9], temp4[9];
				int tno1, tno2, tno3, tno4;
				//复制m和no
				for (int k = 0; k < 9; k++)
				{
					temp1[k] = m1[k];
					temp2[k] = m2[k];
					temp3[k] = m3[k];
					temp4[k] = m4[k];
				}
				tno1 = no1;
				tno2 = no2;
				tno3 = no3;
				tno4 = no4;
				//换位置
				for (int k = 0; k < 9; k++)
				{
					m1[k] = temp2[k];
					m2[k] = temp1[k];
					m3[k] = temp4[k];
					m4[k] = temp3[k];
				}
				no1 = tno2;
				no2 = tno1;
				no3 = tno4;
				no4 = tno3;
			}
			//std::cout << "===== no1=" << no1 << " no2=" << no2 << " no3=" << no3 << " no4=" << no4 << "\n";
			//两个向量相等约束m4-m3==m2-m1
			if (no1<0 || no1>shape_num_)
			 continue;
			if (no2<0 || no1>shape_num_)
				continue;
			if (no3<0 || no1>shape_num_)
				continue;
			if (no4<0 || no1>shape_num_)
				continue;
			cv::Mat constraints_mat_ = cv::Mat::zeros(1, shape_num_ * 6, CV_64F);
			constraints_mat_.at<double>(0, no1 * 3 + 0) += 1; constraints_mat_.at<double>(0, no1 * 3 + 1) += 1; constraints_mat_.at<double>(0, no1 * 3 + 2) += 1;
			constraints_mat_.at<double>(0, no2 * 3 + 0) += -1; constraints_mat_.at<double>(0, no2 * 3 + 1) += -1; constraints_mat_.at<double>(0, no2 * 3 + 2) += -1;
			constraints_mat_.at<double>(0, no3 * 3 + 0) += -1; constraints_mat_.at<double>(0, no3 * 3 + 1) += -1; constraints_mat_.at<double>(0, no3 * 3 + 2) += -1;
			constraints_mat_.at<double>(0, no4 * 3 + 0) += 1; constraints_mat_.at<double>(0, no4 * 3 + 1) += 1; constraints_mat_.at<double>(0, no4 * 3 + 2) = 1;
			constraints_mat_.at<double>(0, shape_num_ * 3 + no1 * 3 + 0) += m1[0] + m1[3] + m1[6];    constraints_mat_.at<double>(0, shape_num_ * 3 + no1 * 3 + 1) += m1[1] + m1[4] + m1[7];    constraints_mat_.at<double>(0, shape_num_ * 3 + no1 * 3 + 2) += m1[2] + m1[5] + m1[8];
			constraints_mat_.at<double>(0, shape_num_ * 3 + no2 * 3 + 0) += -(m2[0] + m2[3] + m2[6]); constraints_mat_.at<double>(0, shape_num_ * 3 + no2 * 3 + 1) += -(m2[1] + m2[4] + m2[7]); constraints_mat_.at<double>(0, shape_num_ * 3 + no2 * 3 + 2) += -(m2[2] + m2[5] + m2[8]);
			constraints_mat_.at<double>(0, shape_num_ * 3 + no3 * 3 + 0) += -(m3[0] + m3[3] + m3[6]); constraints_mat_.at<double>(0, shape_num_ * 3 + no3 * 3 + 1) += -(m3[1] + m3[4] + m3[7]); constraints_mat_.at<double>(0, shape_num_ * 3 + no3 * 3 + 2) += -(m3[2] + m3[5] + m3[8]);
			constraints_mat_.at<double>(0, shape_num_ * 3 + no4 * 3 + 0) += m4[0] + m4[3] + m4[6];    constraints_mat_.at<double>(0, shape_num_ * 3 + no4 * 3 + 1) += m4[1] + m4[4] + m4[7];    constraints_mat_.at<double>(0, shape_num_ * 3 + no4 * 3 + 2) += m4[2] + m4[5] + m4[8];
			spacingConstraints_mats_.push_back(constraints_mat_);
		}
	}
}

cv::Mat CLayoutRefiner::getSpacingUpperPartMatrixA()
{
	//cv::Mat matA = cv::Mat::zeros(numConstraint, numVariable, CV_64F);
	transformM_ = cv::Mat::zeros(shape_num_ * 3, shape_num_ * 3, CV_64F);
	shape_sizeM_ = cv::Mat::zeros(shape_num_ * 3, shape_num_ * 3, CV_64F);

	double matrixMM[3][3];
	matrixMM[0][0] = m_[0] * m_[0] + m_[4] * m_[4]; matrixMM[0][1] = m_[0] * m_[1] + m_[4] * m_[5]; matrixMM[0][2] = m_[0] * m_[2] + m_[4] * m_[6];
	matrixMM[1][0] = m_[0] * m_[1] + m_[4] * m_[5]; matrixMM[1][1] = m_[1] * m_[1] + m_[5] * m_[5]; matrixMM[1][2] = m_[1] * m_[2] + m_[5] * m_[6];
	matrixMM[2][0] = m_[0] * m_[2] + m_[4] * m_[6]; matrixMM[2][1] = m_[1] * m_[2] + m_[5] * m_[6]; matrixMM[2][2] = m_[2] * m_[2] + m_[6] * m_[6];

	for (int i = 0; i < shape_num_; i++)
	{
		transformM_.at<double>(0 + i * 3, 0 + i * 3) = matrixMM[0][0];	transformM_.at<double>(0 + i * 3, 1 + i * 3) = matrixMM[0][1];	transformM_.at<double>(0 + i * 3, 2 + i * 3) = matrixMM[0][2];
		transformM_.at<double>(1 + i * 3, 0 + i * 3) = matrixMM[1][0];  transformM_.at<double>(1 + i * 3, 1 + i * 3) = matrixMM[1][1]; transformM_.at<double>(1 + i * 3, 2 + i * 3) = matrixMM[1][2];
		transformM_.at<double>(2 + i * 3, 0 + i * 3) = matrixMM[2][0];  transformM_.at<double>(2 + i * 3, 1 + i * 3) = matrixMM[2][1]; transformM_.at<double>(2 + i * 3, 2 + i * 3) = matrixMM[2][2];
	}
	for (int i = 0; i < shape_num_ * 3; i++)
	{
		shape_sizeM_.at<double>(i, i) = 10;//1 size
	}
	////固定shape的size约束乘以2000
	//for (int i = 0; i < first_3d_position_location.size(); i++)
	//{
	//	int no = first_3d_position_location[i];
	//	shape_sizeM_.at<double>(no * 3 + 0, no * 3 + 0) = 20000;//1 
	//	shape_sizeM_.at<double>(no * 3 + 1, no * 3 + 1) = 20000;//
	//	shape_sizeM_.at<double>(no * 3 + 2, no * 3 + 2) = 20000;//
	//}
	//
	upper_part_matrixA_ = cv::Mat::zeros(shape_num_ * 3 + 3 * first_3d_position_location.size(), shape_num_ * 6, CV_64F);
	/*for (int i = 0; i < shape_num_ * 3; i++)
	{
		for (int j = 0; j < shape_num_ * 3; j++)
		{
			upper_part_matrixA_.at<double>(i, j) = transformM_.at<double>(i, j);

		}
	}
*/
	//
	for (int i = 0; i < shape_num_ * 3; i++)
	{
		for (int j = 0; j < shape_num_ * 3; j++)
		{
			upper_part_matrixA_.at<double>(i, j + shape_num_ * 3) = shape_sizeM_.at<double>(i, j);
		}
	}
	for (int i = 0; i < first_3d_position_location.size(); i++)
	{
		int no = first_3d_position_location[i];
		upper_part_matrixA_.at<double>(shape_num_ * 3+ i * 3, no * 3 + 0) = 1;//1 20000
		upper_part_matrixA_.at<double>(shape_num_ *3 + i * 3 + 1, no * 3 + 1) = 1;//
		upper_part_matrixA_.at<double>(shape_num_ * 3 + i * 3 + 2, no * 3 + 2) = 1;//
	}
	return upper_part_matrixA_;
}

cv::Mat CLayoutRefiner::getSpacingMatB()
{
	mat_B = cv::Mat::zeros(upper_part_matrixA_.rows + constraints_mats_.size() * 3 + spacingConstraints_mats_.size(), 1, CV_64F);
	/*for (int i = 0; i < point_2d.size(); i++)
	{
	double a = point_2d[i][0];
	double b = point_2d[i][1];
	double vec_b[3];
	vec_b[0] = a*m_[0] - m_[0] * m_[3] + b*m_[4] - m_[4] * m_[7];
	vec_b[1] = a*m_[1] - m_[1] * m_[3] + b*m_[5] - m_[5] * m_[7];
	vec_b[2] = a*m_[2] - m_[2] * m_[3] + b*m_[6] - m_[6] * m_[7];
	mat_B.at<double>(i * 3 + 0, 0) = vec_b[0];
	mat_B.at<double>(i * 3 + 1, 0) = vec_b[1];
	mat_B.at<double>(i * 3 + 2, 0) = vec_b[2];
	}*/
	for (int i = 0; i < shape_num_; i++)
	{
		mat_B.at<double>(shape_num_ * 0 + i * 3 + 0, 0) = shapes_size_[i][0] * 10;
		mat_B.at<double>(shape_num_ * 0 + i * 3 + 1, 0) = shapes_size_[i][1] * 10;
		mat_B.at<double>(shape_num_ * 0 + i * 3 + 2, 0) = shapes_size_[i][2] * 10;
	}
	//shape size约束乘以2000
	/*for (int i = 0; i < first_3d_position_.size(); i++)
	{
	int no = first_3d_position_location[i];
	mat_B.at<double>(shape_num_ * 3 + no * 3 + 0, 0) = mat_B.at<double>(shape_num_ * 3 + no * 3 + 0, 0);
	mat_B.at<double>(shape_num_ * 3 + no * 3 + 1, 0) = mat_B.at<double>(shape_num_ * 3 + no * 3 + 1, 0);
	mat_B.at<double>(shape_num_ * 3 + no * 3 + 2, 0) = mat_B.at<double>(shape_num_ * 3 + no * 3 + 2, 0);
	}*/
	//
	for (int i = 0; i < first_3d_position_.size(); i++)
	{
		mat_B.at<double>(3 * shape_num_ + i * 3 + 0, 0) = first_3d_position_[i][0];//
		mat_B.at<double>(3 * shape_num_ + i * 3 + 1, 0) = first_3d_position_[i][1];
		mat_B.at<double>(3 * shape_num_ + i * 3 + 2, 0) = first_3d_position_[i][2];
	}
	return mat_B;
}





