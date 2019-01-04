#include "ShapePart.h"


CShapePart::CShapePart()
{
	InitializeData();
	InitializeFrame();
}

CShapePart::CShapePart(const CShapePart& shapePart)
{
	m_mesh = new trimesh::TriMesh();
	*m_mesh = *shapePart.m_mesh;
	m_frame = new qglviewer::ManipulatedFrame();

	*m_frame = *shapePart.m_frame;
	m_frame->setReferenceFrame(shapePart.m_frame->referenceFrame());
	m_partColor = shapePart.m_partColor;
	m_isFloating = shapePart.m_isFloating;
	edges = shapePart.edges;
	m_sharpConer = shapePart.m_sharpConer;
	for (int i = 0; i < 6;i++)
	{
		bboxFaces[i] = shapePart.bboxFaces[i];
	}
	m_centerPoint = shapePart.m_centerPoint;
	m_centerPointProjected = shapePart.m_centerPointProjected;
	m_shapeSize = shapePart.m_shapeSize;
	m_optimizing_index = shapePart.m_optimizing_index;
	m_isRotate = shapePart.m_isRotate;
	m_id = shapePart.m_id;
}


CShapePart::~CShapePart()
{
	DestroyData();
}

void CShapePart::InitializeData()
{
	m_frame = NULL;
	m_mesh = NULL;
	m_isFloating = false;
	m_id = -1;
}

void CShapePart::DestroyData()
{
	if (m_frame != NULL) delete m_frame;
	if (m_mesh != NULL) delete m_mesh;
}

void CShapePart::LoadPart(const char* shapePartName)
{
	if (m_mesh)
	{
		delete m_mesh;
	}
	m_mesh = NULL;
	trimesh::TriMesh* thisMesh = trimesh::TriMesh::read(shapePartName);
	thisMesh->need_bsphere();
	thisMesh->need_bbox();
	thisMesh->need_faces();
	thisMesh->need_across_edge();
	thisMesh->need_adjacentfaces();
	thisMesh->need_normals();
	thisMesh->need_tstrips();
	thisMesh->need_neighbors();
	thisMesh->need_curvatures();
	thisMesh->need_dcurv();
	thisMesh->need_pointareas();
	m_mesh = thisMesh;
	//加载模型后的颜色
	m_partColor = QColor(255, 0, 0);
}

void CShapePart::InitializeFrame()
{
	if (m_frame != NULL) delete m_frame;
	m_frame = new qglviewer::ManipulatedFrame();
	m_frame->setSpinningSensitivity(1000.0);
	m_frame->setZoomSensitivity(2);
}

void CShapePart::DrawPart(QColor modelColor)
{
	if (m_frame == NULL || m_mesh == NULL) return;
	glEnable(GL_LIGHTING);
	glPushMatrix();
	//mesh绑定frame
	if (m_isFloating)
		glColor4ub(150, 255, 132, 130);//130 160//glColor4ub(modelColor.red(), modelColor.green(), modelColor.blue(), 150);
	else
		glColor4ub(150, 255, 132, 255);//glColor4ub(108, 255, 132, 255);
	if (m_isFloating)
	{
		if (m_frame->referenceFrame() != NULL)
		{
			qglviewer::Frame lv_frame = *m_frame->referenceFrame();
			glMultMatrixd(lv_frame.matrix());
			double matrix[16];
			for (int i = 0; i < 16; i++)
			{
				matrix[i] = lv_frame.matrix()[i];
			}
			matrix[12] = 0;
			matrix[13] = 0;
			matrix[14] = 0;
			cv::Mat mat_m = cv::Mat::zeros(4, 4, CV_64F);
			for (int m = 0; m < 4; m++)
			{
				for (int n = 0; n < 4; n++)
				{
					mat_m.at<double>(m, n) = matrix[m * 4 + n];
				}
			}
			mat_m = mat_m.inv();
			for (int m = 0; m < 4; m++)
			{
				for (int n = 0; n < 4; n++)
				{
					matrix[m * 4 + n] = mat_m.at<double>(m, n);
				}
			}
			if (gdv_isRotateWithCamera)
			{
				glMultMatrixd(m_frame->matrix());
				glMultMatrixd(matrix);
			}
			else
			{
				glMultMatrixd(m_frame->matrix());
			}

		}
	}
	else
		glMultMatrixd(m_frame->matrix()); //绑定帧，用于旋转和移动
	glShadeModel(GL_SMOOTH);
	glBegin(GL_TRIANGLES);
	for (unsigned int i = 0; i < m_mesh->faces.size(); i++)
	{
		trimesh::TriMesh::Face* thisFace;
		thisFace = &(m_mesh->faces[i]);

		trimesh::vec& n0 = m_mesh->normals[(*thisFace)[0]];
		trimesh::vec& v0 = m_mesh->vertices[(*thisFace)[0]];
		trimesh::vec& n1 = m_mesh->normals[(*thisFace)[1]];
		trimesh::vec& v1 = m_mesh->vertices[(*thisFace)[1]];
		trimesh::vec& n2 = m_mesh->normals[(*thisFace)[2]];
		trimesh::vec& v2 = m_mesh->vertices[(*thisFace)[2]];
		//求法线
		cv::Vec3d p1, p2, p3;
		p1[0] = v0[0];
		p1[1] = v0[1];
		p1[2] = v0[2];
		p2[0] = v1[0];
		p2[1] = v1[1];
		p2[2] = v1[2];
		p3[0] = v2[0];
		p3[1] = v2[1];
		p3[2] = v2[2];
		cv::Vec3d u = p1 - p2;
		cv::Vec3d v = p2 - p3;
		cv::Vec3d normal = Utils::Nomalize(u.cross(v));
		
		glNormal3d(n0[0], n0[1], n0[2]);
		glVertex3f(v0[0], v0[1], v0[2]);

		
		glNormal3d(n0[0], n0[1], n0[2]);
		glVertex3f(v1[0], v1[1], v1[2]);

		
		glNormal3d(n0[0], n0[1], n0[2]);
		glVertex3f(v2[0], v2[1], v2[2]);
		//std::cout << "normal is " << n2[0] << " " << n2[1] << " " << n2[2] << "\n";
	}
	glEnd();
	glPopMatrix();
}

void CShapePart::DrawPart()
{
	DrawPart(m_partColor);
}

void CShapePart::DrawPart(QPainter& painter)
{
	QPen thisPen;
	thisPen.setColor(QColor(0, 0, 255, 255));
	thisPen.setStyle(Qt::CustomDashLine);
	thisPen.setWidthF(2);

	QVector<qreal> dashes;
	qreal space = 4;
	dashes << 2 << space << 2 << space;

	thisPen.setDashPattern(dashes);
	//thisPen.
	painter.save();
	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.setPen(thisPen);
	for (int j = 0; j < edges.size(); j++)
	{
		if (edges[j].m_isVisible)
		{
			edges[j].DrawEdge(painter);
		}
	}
	painter.restore();
}

void CShapePart::DrawPart(qglviewer::Camera* c)
{
	if (m_frame == NULL || m_mesh == NULL) return;
	glEnable(GL_LIGHTING);
	glPushMatrix();
	//mesh绑定frame
	glColor4ub(150, 255, 132, 255);//glColor4ub(108, 255, 132, 255);
	/*if (gdv_isRotateWithCamera)
	{
		qglviewer::Frame lv_frame = *gdv_displaywidgetCamera.frame();
		double matrix[16];
		for (int i = 0; i < 16; i++)
		{
			matrix[i] = lv_frame.matrix()[i];
		}
		matrix[12] = 0;
		matrix[13] = 0;
		matrix[14] = 0;
		cv::Mat mat_m = cv::Mat::zeros(4, 4, CV_64F);
		for (int m = 0; m < 4; m++)
		{
			for (int n = 0; n < 4; n++)
			{
				mat_m.at<double>(m, n) = matrix[m * 4 + n];
			}
		}
		mat_m = mat_m.inv();
		for (int m = 0; m < 4; m++)
		{
			for (int n = 0; n < 4; n++)
			{
				matrix[m * 4 + n] = mat_m.at<double>(m, n);
			}
		}
		
		if (gdv_isRotateWithCamera)
		{
			glMultMatrixd(m_frame->matrix());
			glMultMatrixd(matrix);
		}
		else
		{
			glMultMatrixd(m_frame->matrix());
		}
	}
	else*/
	glMultMatrixd(m_frame->matrix()); //绑定帧，用于旋转和移动
	glShadeModel(GL_SMOOTH);
	glBegin(GL_TRIANGLES);
	for (unsigned int i = 0; i < m_mesh->faces.size(); i++)
	{



		trimesh::TriMesh::Face* thisFace;
		thisFace = &(m_mesh->faces[i]);

		trimesh::vec& n0 = m_mesh->normals[(*thisFace)[0]];
		trimesh::vec& v0 = m_mesh->vertices[(*thisFace)[0]];
		trimesh::vec& n1 = m_mesh->normals[(*thisFace)[1]];
		trimesh::vec& v1 = m_mesh->vertices[(*thisFace)[1]];
		trimesh::vec& n2 = m_mesh->normals[(*thisFace)[2]];
		trimesh::vec& v2 = m_mesh->vertices[(*thisFace)[2]];
		//求法线
		cv::Vec3d p1, p2, p3;
		p1[0] = v0[0];
		p1[1] = v0[1];
		p1[2] = v0[2];
		p2[0] = v1[0];
		p2[1] = v1[1];
		p2[2] = v1[2];
		p3[0] = v2[0];
		p3[1] = v2[1];
		p3[2] = v2[2];
		cv::Vec3d u = p1 - p2;
		cv::Vec3d v = p2 - p3;
		cv::Vec3d normal = Utils::Nomalize(u.cross(v));

		glNormal3d(n0[0], n0[1], n0[2]);
		glVertex3f(v0[0], v0[1], v0[2]);


		glNormal3d(n0[0], n0[1], n0[2]);
		glVertex3f(v1[0], v1[1], v1[2]);


		glNormal3d(n0[0], n0[1], n0[2]);
		glVertex3f(v2[0], v2[1], v2[2]);

	}
	glEnd();
	glPopMatrix();
}

void CShapePart::NormalizeMesh()
{
	if (m_mesh == NULL) return;
	m_mesh->need_bsphere();
	trimesh::TriMesh::BSphere thisBSphere = m_mesh->bsphere;
	trimesh::trans(m_mesh, -thisBSphere.center);
	trimesh::scale(m_mesh, 0.5 / thisBSphere.r);
}

void CShapePart::ComputeGlobalNormal(qglviewer::Camera* c)
{
	//初始化
	for (int i = 0; i < m_sharpConer.size(); i++)
	{
		m_sharpConer[i].m_lbv_isVisible = false;
	}
	//shape的中心点以及相机的视角方向
	qglviewer::Vec center = m_frame->inverseCoordinatesOf(qglviewer::Vec(0, 0, 0));
	cv::Vec3d center_position;
	center_position[0] = center[0];
	center_position[1] = center[1];
	center_position[2] = center[2];
	cv::Vec3d camara_orientation;
	camara_orientation[0] = c->viewDirection()[0];
	camara_orientation[1] = c->viewDirection()[1];
	camara_orientation[2] = c->viewDirection()[2];
	//计算6个面的法线
	bboxFaces[0].vertexIndex[0] = 0; bboxFaces[0].vertexIndex[1] = 1; bboxFaces[0].vertexIndex[2] = 2; bboxFaces[0].vertexIndex[3] = 3; bboxFaces[0].centerPoint = (m_sharpConer[1].pt2D + m_sharpConer[3].pt2D) / 2; bboxFaces[0].center3DPoint = (m_sharpConer[1].pt3D + m_sharpConer[3].pt3D) / 2;
	bboxFaces[1].vertexIndex[0] = 2; bboxFaces[1].vertexIndex[1] = 3; bboxFaces[1].vertexIndex[2] = 7; bboxFaces[1].vertexIndex[3] = 6; bboxFaces[1].centerPoint = (m_sharpConer[3].pt2D + m_sharpConer[6].pt2D) / 2; bboxFaces[1].center3DPoint = (m_sharpConer[3].pt3D + m_sharpConer[6].pt3D) / 2;
	bboxFaces[2].vertexIndex[0] = 4; bboxFaces[2].vertexIndex[1] = 5; bboxFaces[2].vertexIndex[2] = 6; bboxFaces[2].vertexIndex[3] = 7; bboxFaces[2].centerPoint = (m_sharpConer[5].pt2D + m_sharpConer[7].pt2D) / 2; bboxFaces[2].center3DPoint = (m_sharpConer[5].pt3D + m_sharpConer[7].pt3D) / 2;
	bboxFaces[3].vertexIndex[0] = 0; bboxFaces[3].vertexIndex[1] = 1; bboxFaces[3].vertexIndex[2] = 5; bboxFaces[3].vertexIndex[3] = 4; bboxFaces[3].centerPoint = (m_sharpConer[0].pt2D + m_sharpConer[5].pt2D) / 2; bboxFaces[3].center3DPoint = (m_sharpConer[0].pt3D + m_sharpConer[5].pt3D) / 2;
	bboxFaces[4].vertexIndex[0] = 1; bboxFaces[4].vertexIndex[1] = 2; bboxFaces[4].vertexIndex[2] = 6; bboxFaces[4].vertexIndex[3] = 5; bboxFaces[4].centerPoint = (m_sharpConer[2].pt2D + m_sharpConer[5].pt2D) / 2; bboxFaces[4].center3DPoint = (m_sharpConer[2].pt3D + m_sharpConer[5].pt3D) / 2;
	bboxFaces[5].vertexIndex[0] = 0; bboxFaces[5].vertexIndex[1] = 3; bboxFaces[5].vertexIndex[2] = 7; bboxFaces[5].vertexIndex[3] = 4; bboxFaces[5].centerPoint = (m_sharpConer[0].pt2D + m_sharpConer[7].pt2D) / 2; bboxFaces[5].center3DPoint = (m_sharpConer[0].pt3D + m_sharpConer[7].pt3D) / 2;
	for (int i = 0; i < 6; i++)
	{
		cv::Vec3d p0 = m_sharpConer[bboxFaces[i].vertexIndex[0]].pt3D;
		cv::Vec3d p1 = m_sharpConer[bboxFaces[i].vertexIndex[1]].pt3D;
		cv::Vec3d p2 = m_sharpConer[bboxFaces[i].vertexIndex[2]].pt3D;
		cv::Vec3d a = p1 - p0;
		cv::Vec3d b = p2 - p0;
		cv::Vec3d nomal = a.cross(b);
		cv::Vec3d vecTestNomal = m_sharpConer[bboxFaces[i].vertexIndex[0]].pt3D - center_position;
		if (vecTestNomal.dot(nomal) < 0)
		{
			nomal = -nomal;
		}
		if (nomal.dot(camara_orientation) < 0)
		{
			m_sharpConer[bboxFaces[i].vertexIndex[0]].m_lbv_isVisible = true;
			m_sharpConer[bboxFaces[i].vertexIndex[1]].m_lbv_isVisible = true;
			m_sharpConer[bboxFaces[i].vertexIndex[2]].m_lbv_isVisible = true;
			m_sharpConer[bboxFaces[i].vertexIndex[3]].m_lbv_isVisible = true;
		}
	}
}

void CShapePart::ComputeBoxEdgeAndNormal(qglviewer::Camera* c)
{
	if (!m_mesh)
	{
		return;
	}

	m_sharpConer.clear();
	m_centerPoint[0] = m_mesh->bbox.center()[0];
	m_centerPoint[1] = m_mesh->bbox.center()[1];
	m_centerPoint[2] = m_mesh->bbox.center()[2];
	m_centerPointProjected = ComputeProjectedVertex(c,m_frame,m_centerPoint);//中心点的投影
	m_centerPoint = ComputeWorldVertex(c, m_frame, m_centerPoint);
	m_shapeSize[0] = m_mesh->bbox.size()[0];
	m_shapeSize[1] = m_mesh->bbox.size()[1];
	m_shapeSize[2] = m_mesh->bbox.size()[2];
	//计算坐标轴的世界坐标方向
	//m_asixXdir = cv::Vec3d(1,0,0);
	//m_asixYdir = cv::Vec3d(0, 1, 0);
	//m_asixZdir = cv::Vec3d(0, 0, 1);
	//m_asixXdir = ComputeWorldVertex(c, m_frame, m_asixXdir);
	//m_asixYdir = ComputeWorldVertex(c, m_frame, m_asixYdir);
	//m_asixZdir = ComputeWorldVertex(c, m_frame, m_asixZdir);
	//
	//构建shape的6个顶点以及所有边
	cv::Vec3d maxPoint, minPoint;
	maxPoint[0] = m_mesh->bbox.max[0];
	maxPoint[1] = m_mesh->bbox.max[1];
	maxPoint[2] = m_mesh->bbox.max[2];
	minPoint[0] = m_mesh->bbox.min[0];
	minPoint[1] = m_mesh->bbox.min[1];
	minPoint[2] = m_mesh->bbox.min[2];

	//使得bbox保证是一个标准的长方体
	//x
	if (abs(maxPoint[0]) < abs(minPoint[0]))
	{
		maxPoint[0] = abs(minPoint[0]);
	}
	else
	{
		minPoint[0] = -abs(maxPoint[0]);
	}
	//y
	if (abs(maxPoint[1]) < abs(minPoint[1]))
	{
		maxPoint[1] = abs(minPoint[1]);
	}
	else
	{
		minPoint[1] = -abs(maxPoint[1]);
	}
	//z
	if (abs(maxPoint[2]) < abs(minPoint[2]))
	{
		maxPoint[2] = abs(minPoint[2]);
	}
	else
	{
		minPoint[2] = -abs(maxPoint[2]);
	}
	//6个顶点
	cv::Vec3d p1, p2, p3, p4, p5, p6, p7, p8;
	p1 = p2 = p4 = p5 = minPoint;
	p7 = p6 = p8 = p3 = maxPoint;
	p2[1] = -p2[1];
	p4[0] = -p4[0];
	p5[2] = -p5[2];
	p6[0] = -p6[0];
	p8[1] = -p8[1];
	p3[2] = -p3[2];
	//
	if (m_isFloating&&gdv_isRotateWithCamera)
	{
		double matrix[16];
		double camera_matrix[16];
		c->frame()->getMatrix(camera_matrix);
		for (int i = 0; i < 16; i++)
		{
			matrix[i] = camera_matrix[i];
		}
		matrix[12] = 0;
		matrix[13] = 0;
		matrix[14] = 0;
		p1 = Utils::MatrixMult31(matrix, p1);
		p2 = Utils::MatrixMult31(matrix, p2);
		p3 = Utils::MatrixMult31(matrix, p3);
		p4 = Utils::MatrixMult31(matrix, p4);
		p5 = Utils::MatrixMult31(matrix, p5);
		p6 = Utils::MatrixMult31(matrix, p6);
		p7 = Utils::MatrixMult31(matrix, p7);
		p8 = Utils::MatrixMult31(matrix, p8);
	}
	//
	std::vector<cv::Vec3d> vertices;
	vertices.push_back(p1); vertices.push_back(p2); vertices.push_back(p3); vertices.push_back(p4);
	vertices.push_back(p5); vertices.push_back(p6); vertices.push_back(p7); vertices.push_back(p8);
	//m_sharpConer
	for (int i = 0; i < vertices.size(); i++)
	{
		cv::Vec3d c_Pt3d;
		c_Pt3d[0] = vertices[i][0]; c_Pt3d[1] = vertices[i][1]; c_Pt3d[2] = vertices[i][2];
		SharpConer sharpCorner;
		qglviewer::Vec globalPos, localPos, projectedPos;
		localPos.x = c_Pt3d[0];
		localPos.y = c_Pt3d[1];
		localPos.z = c_Pt3d[2];
		globalPos = this->m_frame->inverseCoordinatesOf(localPos);
		cv::Vec3d point;
		point[0] = globalPos.x;
		point[1] = globalPos.y;
		point[2] = globalPos.z;
		sharpCorner.pt3D = point;
		projectedPos = c->projectedCoordinatesOf(globalPos);
		QPointF thisPt;
		thisPt.setX(projectedPos.x);
		thisPt.setY(projectedPos.y);
		sharpCorner.pt2D[0] = thisPt.x();
		sharpCorner.pt2D[1] = thisPt.y();
		m_sharpConer.push_back(sharpCorner);
	}
	std::vector<cv::Vec3d> points_3d;
	points_3d.push_back(p1);
	points_3d.push_back(p2);
	points_3d.push_back(p3);
	points_3d.push_back(p4);
	points_3d.push_back(p5);
	points_3d.push_back(p6);
	points_3d.push_back(p7);
	points_3d.push_back(p8);
	edges.clear();
	Edge e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, e11, e12;
	e1.SetEdge(c, this->m_frame, points_3d[0], points_3d[1], 0, 0,minPoint,maxPoint); edges.push_back(e1);
	e2.SetEdge(c, this->m_frame, points_3d[0], points_3d[4], 0, 1, minPoint, maxPoint); edges.push_back(e2);
	e3.SetEdge(c, this->m_frame, points_3d[0], points_3d[3], 0, 2, minPoint, maxPoint); edges.push_back(e3);
	e4.SetEdge(c, this->m_frame, points_3d[4], points_3d[5], 1, 3, minPoint, maxPoint); edges.push_back(e4);
	e5.SetEdge(c, this->m_frame, points_3d[1], points_3d[5], 1, 4, minPoint, maxPoint); edges.push_back(e5);
	e6.SetEdge(c, this->m_frame, points_3d[4], points_3d[7], 1, 5, minPoint, maxPoint); edges.push_back(e6);
	e7.SetEdge(c, this->m_frame, points_3d[7], points_3d[6], 0, 6, minPoint, maxPoint); edges.push_back(e7);
	e8.SetEdge(c, this->m_frame, points_3d[2], points_3d[6], 0, 7, minPoint, maxPoint); edges.push_back(e8);
	e9.SetEdge(c, this->m_frame, points_3d[5], points_3d[6], 0, 8, minPoint, maxPoint); edges.push_back(e9);
	e10.SetEdge(c, this->m_frame, points_3d[3], points_3d[2], 1, 9, minPoint, maxPoint); edges.push_back(e10);
	e11.SetEdge(c, this->m_frame, points_3d[3], points_3d[7], 1, 10, minPoint, maxPoint); edges.push_back(e11);
	e12.SetEdge(c, this->m_frame, points_3d[1], points_3d[2], 1, 11, minPoint, maxPoint); edges.push_back(e12);
	ComputeGlobalNormal(c);	//计算法线
	//初始化，全部边可见
	for (int i = 0; i < edges.size();i++)
	{
		edges[i].m_isVisible = true;
	}
	//检测边是否可见
	for (int i = 0; i < edges.size(); i++)
	{
		cv::Vec2d p1, p2;
		p1 = edges[i].m_projectPt2D[0];
		p2 = edges[i].m_projectPt2D[1];
		for (int j = 0; j < m_sharpConer.size(); j++)
		{
			if (abs(m_sharpConer[j].pt2D[0] - p1[0]) < 0.0001 && abs(m_sharpConer[j].pt2D[1] - p1[1]) < 0.0001)
			{
				if (!m_sharpConer[j].m_lbv_isVisible)
				{
					edges[i].m_isVisible = false;
					break;
				}
			}
			if (abs(m_sharpConer[j].pt2D[0] - p2[0]) < 0.0001 && abs(m_sharpConer[j].pt2D[1] - p2[1]) < 0.0001)
			{
				if (!m_sharpConer[j].m_lbv_isVisible)
				{
					edges[i].m_isVisible = false;
					break;
				}
			}

		}
	}
	//shape id and edge id

	for (int j = 0; j < edges.size(); j++)
	{
		edges[j].m_shapeId =m_id;
		edges[j].m_edgeId = j;
	}
	
}

cv::Vec2d CShapePart::ComputeProjectedVertex(qglviewer::Camera* c, qglviewer::ManipulatedFrame* frame, cv::Vec3d Pt3D)
{
	qglviewer::Vec globalPos, localPos, projectedPos;
	localPos.x = Pt3D[0];
	localPos.y = Pt3D[1];
	localPos.z = Pt3D[2];
	globalPos = frame->inverseCoordinatesOf(localPos);
	projectedPos = c->projectedCoordinatesOf(globalPos);
	cv::Vec2d thisPt;
	thisPt[0] = projectedPos.x;
	thisPt[1] = projectedPos.y;
	return thisPt;
}

cv::Vec3d CShapePart::ComputeWorldVertex(qglviewer::Camera* c, qglviewer::ManipulatedFrame* frame, cv::Vec3d Pt3D)
{
	qglviewer::Vec globalPos, localPos;
	localPos.x = Pt3D[0];
	localPos.y = Pt3D[1];
	localPos.z = Pt3D[2];
	globalPos = frame->inverseCoordinatesOf(localPos);
	cv::Vec3d thisPt;
	thisPt[0] = globalPos.x;
	thisPt[1] = globalPos.y;
	thisPt[2] = globalPos.z;
	return thisPt;
}

void CShapePart::DrawNo(QPainter& painter)
{
	if (m_id == -1) return;
	QString text = QString::number(m_id, 10);
	QFont font;
	font.setPointSize(20);
	painter.save();
	painter.setFont(font);
	painter.drawText(m_centerPointProjected[0]-10, m_centerPointProjected[1]+10, text);
	painter.restore();
}
