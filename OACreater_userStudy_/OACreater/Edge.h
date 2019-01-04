#pragma once
#include<vector>
#include <ctime>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <QGLViewer/qglviewer.h>
#include <QGLViewer/manipulatedFrame.h>
#include <QtGui>
#include "Utils.h"

class Edge
{
public:
	Edge();
	~Edge();
	void SetEdge(qglviewer::Camera* c, qglviewer::ManipulatedFrame* frame, cv::Vec3d point1, cv::Vec3d point2, int r_no, int shapeEdgeNo, cv::Vec3d bbox_min, cv::Vec3d bbox_max);
public:
	cv::Vec2d ComputeProjectedVertex(qglviewer::Camera* c, qglviewer::ManipulatedFrame* frame, cv::Vec3d Pt3D);
	cv::Vec3d Compute3DLocalToWorld(qglviewer::Camera* c, qglviewer::ManipulatedFrame* frame, cv::Vec3d Pt3D);
	void ComputeEdgePoints();
	void Edge::ComputeEndPt2DProjectedVertex();
	cv::Vec2d GetProjectivePoint(cv::Vec2d pLine, cv::Vec2d pOut);
	void ComputeCenterToVertex(qglviewer::ManipulatedFrame* frame);
	void ComputeCenterToVertex1(qglviewer::ManipulatedFrame* frame);
	void Compute3DTo2DMatrix(qglviewer::Camera* c);
	void DrawEdge(QPainter& painter);
public:
	//3D
	cv::Vec3d m_globalPt3D[2];//世界坐标系
	cv::Vec3d m_localPt3D[2];//局部坐标系
	//2d
	cv::Vec2d m_projectPt2D[2];
	//2D和3D下边的方向
	cv::Vec3d m_dir3D;
	cv::Vec2d m_dir2D;
	bool m_isVisible;//该条边是否可见
	int m_shapeIndex;//该条边属于哪个shape
	int m_allEdgeNo;//所有可见边的一个编号，用于ransac算法里面
	int m_shapeEdgeNo;
	std::vector<cv::Vec2d> m_points;//一条边sample几个点
	//边到边（线）的距离
	float m_edgeToLineDis;
	//一条线的四个参数
	float m_line[4];//线
	cv::Vec2d m_endPt2DPV[2];
	float m_point_dis[2];
	cv::Vec2d m_point_line;
	int m_rotateNo;//旋转优化时用的辅助参数
	//优化的两个矩阵
	double m_3DTo2DMatrix[8];
	double m_centerToVertex[9];
	double m_centerToVertex1[9];
	//bbox的两个点
	cv::Vec3d m_bboxMin;
	cv::Vec3d m_bboxMax;
	////关系编辑时辅助点，不需要赋值
	//cv::Vec2d m_edgeCenterPoint;
	//shape号和edge号记录唯一一条边
	int  m_shapeId;
	int m_edgeId;
	//等距判断依据
	int m_spacingId;
	double m_point_edge_dis;//添加关系时用
};

