#pragma once
#include <stdlib.h>
#include <opencv2/core/core.hpp>
#include "Cholmod_solver_traits.h"
#include "ShapeHandler.h"
#include "Utils.h"
#include "Graph.h"
typedef Cholmod_solver_traits<double> Solver;
struct VecEdges
{
	std::vector<int> shape_indexs;//一条关系的所有shape的编号
	std::vector<std::vector<Edge>> edges_vec;//一条关系的所有边边
};
class CLayoutRefiner
{
private:
	int shape_num_;
	double m_[8];
	vector<cv::Vec3d> first_3d_position_;
	vector<int> first_3d_position_location;
	cv::Mat transformM_;
	cv::Mat shape_sizeM_;
	vector<cv::Vec3d> shapes_size_;
	cv::Mat upper_part_matrixA_;
	cv::Mat mat_B;
	cv::Mat mat_A;
	vector<cv::Vec2d> point_2d;
	vector<vector<double>> reference_Matrix;
	vector<vector<vector<double>>> reference_Matrix_vector;
	vector<vector<vector<vector<double>>>> spacing_Matrix_vector;
	vector<cv::Vec3d> constraint_dir;
public:
	CLayoutRefiner(int shape_size);
	CLayoutRefiner();
	~CLayoutRefiner();
private:
	//set
	void setmatrixM(double m1[8]);
	void setFirst3DPosition(cv::Vec3d point, int index);
	void setShapeSize(cv::Vec3d shape_size);
	void setPosition2D(cv::Vec2d point);
	void setReferenceMatrix(vector<vector<double>> matrix);
	void setSpaingMatrix(vector<vector<vector<double>>> matrix);
	void setConstraint_dir(cv::Vec3d dir);
	//compute
	void computeShapeAndTransformatMatrix();
	void computeMatB();
	void computeConstraints();
	void computeConstraints2();
	void computeSpacingConstraints();
	//get
	cv::Mat getUpperPartMatrixA();
	cv::Mat getSpacingUpperPartMatrixA();
	cv::Mat getMatB();
	cv::Mat getSpacingMatB();
	cv::Mat getMatA();
	//constraints
	vector<cv::Mat> constraints_mats_;
	vector<cv::Mat> spacingConstraints_mats_;
	//传递的参数
private:
	CShapeHandler* m_shapeHandler_h;
	CShapeHandler m_cloneShapeHandlerp;
	//优化
	Solver* my_solver_;
public:
	void SetData(CShapeHandler* shapeHandlerp);
	std::vector<std::vector<Edge>> CheckRelations(std::vector<std::vector<Edge>> relations, qglviewer::Camera* c);
	void ExecuteAlignmentOptimizing(std::vector<ShapeRelation> relations, qglviewer::Camera* c, std::vector<bool>* floatIndex);
	bool CheckErrorRelation(std::vector<ShapeRelation> relations, qglviewer::Camera* c, std::vector<bool> floatIndex);
	bool CheckAndAddRelation(std::vector<ShapeRelation> relations, qglviewer::Camera* c, std::vector<bool> floatIndex, std::vector<ShapeRelation>* availablerelations);
	void RotationOptimizing(std::vector<ShapeRelation>* relations, qglviewer::Camera* c);
	//测试
	void ExecuteSpacingOptimizing(std::vector<ShapeRelation> relations, qglviewer::Camera* c, std::vector<std::vector<RecordNo>> recordNoss);
private:
	//旋转
	void RotateWithOneRelaion(Edge edge_from, Edge edge_to);
	void RotateWithTwoRelaion(Edge edge_from, Edge edge_to);
	qglviewer::Vec Point2Direction(cv::Vec3d endPt3D[2], qglviewer::ManipulatedFrame* frame);
	cv::Vec3d Vec2CVec(qglviewer::Vec v);
	qglviewer::Quaternion GetAxisAngleBy2Vector(qglviewer::Vec from, qglviewer::Vec to);
	void UpdateVecRelations(vector<ShapeRelation> &availableRelations);
	//坐标对齐
	void OneCoordinateAlignment(Edge edge_from, Edge edge_to);
	void AllCoordinateAlignment(Edge edge_from, Edge edge_to);
	void AllCoordinateAlignment(int index1, int index2);
	cv::Vec3d Normalize(cv::Vec3d u);
	//////////////////////////////////////////////////////////////////////////
	double Compute3DDirAngle(cv::Vec3d cv_from, cv::Vec3d cv_to);
	//
	Solver::Vector SetAddConstrisB(Solver::Vector B);
	Solver::Vector SetDeleteConstrisB(Solver::Vector B);

};

