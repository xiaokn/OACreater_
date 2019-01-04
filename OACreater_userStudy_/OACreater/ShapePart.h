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
	//obj文件对应的mesh
	trimesh::TriMesh* m_mesh;
	//mesh绑定的frame，用于控制mesh在世界坐标系中的方向和位置
	qglviewer::ManipulatedFrame* m_frame;
	//shape的渲染颜色
	QColor m_partColor;
	bool m_isFloating;	//该shape的位置是否处于悬浮状态
	std::vector<Edge> edges;
	std::vector<SharpConer> m_sharpConer;
	Face bboxFaces[6];	//一个shape6个面
	cv::Vec3d m_centerPoint;	//shape的中心点
	cv::Vec2d m_centerPointProjected;	//shape的中心点的二维投影
	cv::Vec3d m_shapeSize;	//shape的size
	int m_optimizing_index;
	bool m_isRotate;	//该shape是否可以旋转，用于坐标对齐
	int m_id;	//该shape的唯一id
	/*cv::Vec3d m_asixXdir;
	cv::Vec3d m_asixYdir;
	cv::Vec3d m_asixZdir;*/
};

