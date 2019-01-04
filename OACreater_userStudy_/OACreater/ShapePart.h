#pragma once
#include <TriMesh.h>
#include <TriMesh_algo.h>
#include <QGLViewer/qglviewer.h>
#include <QGLViewer/manipulatedFrame.h>
#include <QGLViewer/camera.h>
#include <QGLViewer/manipulatedCameraFrame.h>
#include <QtGui>
#include <opencv2/core/core.hpp>
#include "Edge.h"
#include "Utils.h"
class CShapePart
{
public:
	CShapePart();
	CShapePart(const CShapePart& shapePart);
	~CShapePart();
public:
	void InitializeData();
	void DestroyData();
	void LoadPart(const char* shapePartName);
	void InitializeFrame();
	void DrawPart(QColor modelColor);
	void DrawPart(qglviewer::Camera* c);
	void DrawPart();
	void DrawPart(QPainter& painter);
	void DrawNo(QPainter& painter);
	void NormalizeMesh();
	void ComputeGlobalNormal(qglviewer::Camera* c);
	void ComputeBoxEdgeAndNormal(qglviewer::Camera* c);
	cv::Vec2d ComputeProjectedVertex(qglviewer::Camera* c,qglviewer::ManipulatedFrame* frame, cv::Vec3d Pt3D);
	cv::Vec3d ComputeWorldVertex(qglviewer::Camera* c, qglviewer::ManipulatedFrame* frame, cv::Vec3d Pt3D);
public:
	//obj�ļ���Ӧ��mesh
	trimesh::TriMesh* m_mesh;
	//mesh�󶨵�frame�����ڿ���mesh����������ϵ�еķ����λ��
	qglviewer::ManipulatedFrame* m_frame;
	//shape����Ⱦ��ɫ
	QColor m_partColor;
	bool m_isFloating;	//��shape��λ���Ƿ�������״̬
	std::vector<Edge> edges;
	std::vector<SharpConer> m_sharpConer;
	Face bboxFaces[6];	//һ��shape6����
	cv::Vec3d m_centerPoint;	//shape�����ĵ�
	cv::Vec2d m_centerPointProjected;	//shape�����ĵ�Ķ�άͶӰ
	cv::Vec3d m_shapeSize;	//shape��size
	int m_optimizing_index;
	bool m_isRotate;	//��shape�Ƿ������ת�������������
	int m_id;	//��shape��Ψһid
	/*cv::Vec3d m_asixXdir;
	cv::Vec3d m_asixYdir;
	cv::Vec3d m_asixZdir;*/
};

