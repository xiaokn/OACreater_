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
	cv::Vec3d m_globalPt3D[2];//��������ϵ
	cv::Vec3d m_localPt3D[2];//�ֲ�����ϵ
	//2d
	cv::Vec2d m_projectPt2D[2];
	//2D��3D�±ߵķ���
	cv::Vec3d m_dir3D;
	cv::Vec2d m_dir2D;
	bool m_isVisible;//�������Ƿ�ɼ�
	int m_shapeIndex;//�����������ĸ�shape
	int m_allEdgeNo;//���пɼ��ߵ�һ����ţ�����ransac�㷨����
	int m_shapeEdgeNo;
	std::vector<cv::Vec2d> m_points;//һ����sample������
	//�ߵ��ߣ��ߣ��ľ���
	float m_edgeToLineDis;
	//һ���ߵ��ĸ�����
	float m_line[4];//��
	cv::Vec2d m_endPt2DPV[2];
	float m_point_dis[2];
	cv::Vec2d m_point_line;
	int m_rotateNo;//��ת�Ż�ʱ�õĸ�������
	//�Ż�����������
	double m_3DTo2DMatrix[8];
	double m_centerToVertex[9];
	double m_centerToVertex1[9];
	//bbox��������
	cv::Vec3d m_bboxMin;
	cv::Vec3d m_bboxMax;
	////��ϵ�༭ʱ�����㣬����Ҫ��ֵ
	//cv::Vec2d m_edgeCenterPoint;
	//shape�ź�edge�ż�¼Ψһһ����
	int  m_shapeId;
	int m_edgeId;
	//�Ⱦ��ж�����
	int m_spacingId;
	double m_point_edge_dis;//��ӹ�ϵʱ��
};

