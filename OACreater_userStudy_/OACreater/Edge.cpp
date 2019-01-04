#include "Edge.h"


Edge::Edge()
{
	m_isVisible = true;
	m_edgeToLineDis = 0;
	m_spacingId = -1;
	m_point_edge_dis = -1;
}


Edge::~Edge()
{
}

void Edge::SetEdge(qglviewer::Camera* c, qglviewer::ManipulatedFrame* frame, cv::Vec3d point1, cv::Vec3d point2, int r_no, int shapeEdgeNo, cv::Vec3d bbox_min, cv::Vec3d bbox_max)
{
	m_bboxMin = bbox_min;
	m_bboxMax = bbox_max;
	m_shapeEdgeNo = shapeEdgeNo;
	m_rotateNo = r_no;//旋转优化的辅助变量
	//得到3D局部坐标和世界坐标以及3D坐标的投影
	m_localPt3D[0] = point1;
	m_localPt3D[1] = point2;
	m_globalPt3D[0] = Compute3DLocalToWorld(c, frame, point1);
	m_globalPt3D[1] = Compute3DLocalToWorld(c, frame, point2);
	m_projectPt2D[0] = ComputeProjectedVertex(c, frame, point1);
	m_projectPt2D[1] = ComputeProjectedVertex(c, frame, point2);
	ComputeEdgePoints();
	//计算2D和3D的方向
	m_dir2D[0] = m_projectPt2D[1][0] - m_projectPt2D[0][0];
	m_dir2D[1] = m_projectPt2D[1][1] - m_projectPt2D[0][1];
	m_dir3D[0] = m_globalPt3D[1][0] - m_globalPt3D[0][0];
	m_dir3D[1] = m_globalPt3D[1][1] - m_globalPt3D[0][1];
	m_dir3D[2] = m_globalPt3D[1][2] - m_globalPt3D[0][2];
	//单位化
	m_dir2D = Utils::Nomalize(m_dir2D);
	m_dir3D = Utils::Nomalize(m_dir3D);
	Compute3DTo2DMatrix(c);
	ComputeCenterToVertex(frame);
	ComputeCenterToVertex1(frame);

	//计算gdv_unitThreshold
	double threshold = Utils::Calculate2DTwoPointDistance(m_projectPt2D[0],m_projectPt2D[1])/5;
	gdv_unitThreshold = (gdv_unitThreshold + threshold) / 2;

}
cv::Vec2d Edge::ComputeProjectedVertex(qglviewer::Camera* c, qglviewer::ManipulatedFrame* frame, cv::Vec3d Pt3D)
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
	//returnM(c, m);
	return thisPt;
}

cv::Vec3d Edge::Compute3DLocalToWorld(qglviewer::Camera* c, qglviewer::ManipulatedFrame* frame, cv::Vec3d Pt3D)
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

void Edge::ComputeEdgePoints()
{
	m_points.clear();
	float part_lenth = 10;
	float dis = Utils::Calculate2DTwoPointDistance(m_projectPt2D);
	float dx = (m_projectPt2D[1][0] - m_projectPt2D[0][0]) / (dis / part_lenth);
	float dy = (m_projectPt2D[1][1] - m_projectPt2D[0][1]) / (dis / part_lenth);
	m_points.push_back(m_projectPt2D[0]);
	cv::Vec2d start = m_projectPt2D[0];
	for (int i = 0; i < dis / part_lenth - 1; i++)
	{
		start[0] += dx;
		start[1] += dy;
		m_points.push_back(start);
	}
	m_points.push_back(m_projectPt2D[1]);
}

void Edge::ComputeEndPt2DProjectedVertex()
{
	if (m_line[0] == NULL)
	{
		return;
	}
	cv::Vec2d pline;
	pline[0] = m_line[2];
	pline[1] = m_line[3];
	m_endPt2DPV[0] = GetProjectivePoint(pline, m_projectPt2D[0]);
	m_endPt2DPV[1] = GetProjectivePoint(pline, m_projectPt2D[1]);

	if (m_line[0] == 0)
	{
		m_point_dis[0] = m_endPt2DPV[0][1];
		m_point_dis[1] = m_endPt2DPV[1][1];
		return;
	}
	double k;
	k = m_line[1] / m_line[0];
	if (k == 0)
	{
		m_point_dis[0] = m_endPt2DPV[0][0];
		m_point_dis[1] = m_endPt2DPV[1][0];
		return;
	}
	else
	{
		m_point_dis[0] = sqrt(m_endPt2DPV[0][0] * m_endPt2DPV[0][0] + (m_endPt2DPV[0][1] - m_point_line[1])*(m_endPt2DPV[0][1] - m_point_line[1]));
		m_point_dis[1] = sqrt(m_endPt2DPV[1][0] * m_endPt2DPV[1][0] + (m_endPt2DPV[1][1] - m_point_line[1])*(m_endPt2DPV[1][1] - m_point_line[1]));
		return;
	}
}

cv::Vec2d Edge::GetProjectivePoint(cv::Vec2d pLine, cv::Vec2d pOut)
{
	double k;
	cv::Vec2d pProject;
	if (m_line[0] == 0)
	{
		pProject[0] = pLine[0];
		pProject[1] = pOut[1];
		//
		m_point_line[0] = pLine[0];
		m_point_line[1] = 0;
		return pProject;
	}
	k = m_line[1] / m_line[0];
	if (k == 0)
	{
		pProject[0] = pOut[0];
		pProject[1] = pLine[1];
		//
		m_point_line[0] = 0;
		m_point_line[1] = pLine[1];
		return pProject;
	}
	else
	{
		pProject[0] = (float)((k * pLine[0] + pOut[0] / k + pOut[1] - pLine[1]) / (1 / k + k));
		pProject[1] = (float)(-1 / k * (pProject[0] - pOut[0]) + pOut[1]);
		//
		m_point_line[0] = 0;
		m_point_line[1] = m_line[3] - k*m_line[2];
		return pProject;
	}
}

void Edge::ComputeCenterToVertex(qglviewer::ManipulatedFrame* frame)
{
	double a, b, c;
	if (m_localPt3D[0][0] > 0)
	{
		a = 0.5;
		//a = m_bboxMax[0] / sizeX;
	}
	else
	{
		a = -0.5;
		//a = m_bboxMin[0] / sizeX;
	}
	if (m_localPt3D[0][1] > 0)
	{
		b = 0.5;
		//b = m_bboxMax[1] / sizeY;
	}
	else
	{
		b = -0.5;
		//b = m_bboxMin[1] / sizeY;
	}
	if (m_localPt3D[0][2] > 0)
	{
		c = 0.5;
		//c = m_bboxMax[2] / sizeZ;
	}
	else
	{
		c = -0.5;
		//c = m_bboxMin[2] / sizeZ;
	}
	//m是坐标转换矩阵
	GLdouble get_m[16], m[16], rotateM[9];
	frame->getWorldMatrix(get_m);
#define M(row,col) get_m[col*4+row]
	m[0] = M(0, 0); m[1] = M(0, 1); m[2] = M(0, 2); m[3] = M(0, 3);
	m[4] = M(1, 0); m[5] = M(1, 1); m[6] = M(1, 2); m[7] = M(1, 3);
	m[8] = M(2, 0); m[9] = M(2, 1); m[10] = M(2, 2); m[11] = M(2, 3);
	m[12] = M(3, 0); m[13] = M(3, 1); m[14] = M(3, 2); m[15] = M(3, 3);
#undef M
	rotateM[0] = m[0]; rotateM[1] = m[1]; rotateM[2] = m[2];
	rotateM[3] = m[4]; rotateM[4] = m[5]; rotateM[5] = m[6];
	rotateM[6] = m[8]; rotateM[7] = m[9]; rotateM[8] = m[10];
	m_centerToVertex[0] = m[0] * a; m_centerToVertex[1] = m[1] * b; m_centerToVertex[2] = m[2] * c;
	m_centerToVertex[3] = m[4] * a; m_centerToVertex[4] = m[5] * b; m_centerToVertex[5] = m[6] * c;
	m_centerToVertex[6] = m[8] * a; m_centerToVertex[7] = m[9] * b; m_centerToVertex[8] = m[10] * c;
	////test
	//GLdouble out[3];
	//GLdouble in[3];
	//in[0] = m_bboxMax[0]-m_bboxMin[0];
	//in[1] = m_bboxMax[1]-m_bboxMin[1];
	//in[2] = m_bboxMax[2]-m_bboxMin[2];
	//out[0] = m_centerToVertex[0] * in[0] + m_centerToVertex[1] * in[1] + m_centerToVertex[2] * in[2];
	//out[1] = m_centerToVertex[3] * in[0] + m_centerToVertex[4] * in[1] + m_centerToVertex[5] * in[2];
	//out[2] = m_centerToVertex[6] * in[0] + m_centerToVertex[7] * in[1] + m_centerToVertex[8] * in[2];
	//cv::Vec3d center = Compute3DLocalToWorld(NULL, frame, cv::Vec3d(0, 0, 0));
	//std::cout << " 源点  " << m_globalPt3D[0][0] << " " << m_globalPt3D[0][1] << " " << m_globalPt3D[0][2] << " \n";
	//std::cout << " 求的点  " << center[0] + out[0] << " " << center[1] + out[1] << " " << center[2] + out[2] << " \n";
}

void Edge::Compute3DTo2DMatrix(qglviewer::Camera* c)
{
	GLdouble modelMatrix[16];
	GLdouble projMatrix[16];
	GLint viewport[4];
	c->getViewport(viewport);
	c->computeModelViewMatrix();
	c->computeProjectionMatrix();
	c->getModelViewMatrix(modelMatrix);
	c->getProjectionMatrix(projMatrix);
	GLdouble out2[16], out3[16], out4[8];
	GLdouble view[16], *output;
	for (int k = 0; k < 16; k++)
	{
		view[k] = 0;
	}
	double x, y, w, h;
	x = viewport[0];
	y = viewport[1];
	w = viewport[2];
	h = viewport[3];
	view[0] = w / 2;
	view[3] = x + w / 2;
	view[5] = h / 2;
	view[7] = y + h / 2;
	//view[10] = (zFar-zNear)/2;
	//view[11] = (zFar+zNear)/2;
	view[15] = 1;

	for (int k = 0; k < 4; k++){
		for (int s = 0; s < 4; s++){
			out2[k * 4 + s] = 0;//变量使用前记得初始化,否则结果具有不确定性  
			for (int n = 0; n < 4; n++){
				out2[k * 4 + s] += modelMatrix[k * 4 + n] * projMatrix[n * 4 + s];
			}
		}
	}
	for (int k = 0; k < 4; k++){
		for (int s = 0; s < 4; s++){
			out3[k * 4 + s] = 0;//变量使用前记得初始化,否则结果具有不确定性  
			for (int n = 0; n < 4; n++){
				out3[k * 4 + s] += out2[k * 4 + n] * view[n * 4 + s];
			}
		}
	}
	out4[0] = out3[0]; out4[1] = out3[4]; out4[2] = out3[8]; out4[3] = out3[12] + w / 2;
	out4[4] = out3[1]; out4[5] = out3[5]; out4[6] = out3[9]; out4[7] = out3[13] - h / 2;
	for (int i = 0; i < 8; i++)
	{
		m_3DTo2DMatrix[i] = out4[i];
	}
}

void Edge::DrawEdge(QPainter& painter)
{
	QPointF point1, point2;
	point1.setX(m_projectPt2D[0][0]); point1.setY(m_projectPt2D[0][1]);
	point2.setX(m_projectPt2D[1][0]); point2.setY(m_projectPt2D[1][1]);
	painter.drawLine(point1, point2);
}

void Edge::ComputeCenterToVertex1(qglviewer::ManipulatedFrame* frame)
{
	double a, b, c;
	if (m_localPt3D[1][0] > 0)
	{
		a = 0.5;
	}
	else
	{
		a = -0.5;
	}
	if (m_localPt3D[1][1] > 0)
	{
		b = 0.5;
	}
	else
	{
		b = -0.5;
	}
	if (m_localPt3D[1][2] > 0)
	{
		c = 0.5;
	}
	else
	{
		c = -0.5;
	}
	//m是坐标转换矩阵
	GLdouble get_m[16], m[16], rotateM[9];
	frame->getWorldMatrix(get_m);
#define M(row,col) get_m[col*4+row]
	m[0] = M(0, 0); m[1] = M(0, 1); m[2] = M(0, 2); m[3] = M(0, 3);
	m[4] = M(1, 0); m[5] = M(1, 1); m[6] = M(1, 2); m[7] = M(1, 3);
	m[8] = M(2, 0); m[9] = M(2, 1); m[10] = M(2, 2); m[11] = M(2, 3);
	m[12] = M(3, 0); m[13] = M(3, 1); m[14] = M(3, 2); m[15] = M(3, 3);
#undef M
	rotateM[0] = m[0]; rotateM[1] = m[1]; rotateM[2] = m[2];
	rotateM[3] = m[4]; rotateM[4] = m[5]; rotateM[5] = m[6];
	rotateM[6] = m[8]; rotateM[7] = m[9]; rotateM[8] = m[10];
	m_centerToVertex1[0] = m[0] * a; m_centerToVertex1[1] = m[1] * b; m_centerToVertex1[2] = m[2] * c;
	m_centerToVertex1[3] = m[4] * a; m_centerToVertex1[4] = m[5] * b; m_centerToVertex1[5] = m[6] * c;
	m_centerToVertex1[6] = m[8] * a; m_centerToVertex1[7] = m[9] * b; m_centerToVertex1[8] = m[10] * c;
	////test
	//GLdouble out[3];
	//GLdouble in[3];
	//in[0] = m_bboxMax[0] - m_bboxMin[0];
	//in[1] = m_bboxMax[1] - m_bboxMin[1];
	//in[2] = m_bboxMax[2] - m_bboxMin[2];
	//out[0] = m_centerToVertex1[0] * in[0] + m_centerToVertex1[1] * in[1] + m_centerToVertex1[2] * in[2];
	//out[1] = m_centerToVertex1[3] * in[0] + m_centerToVertex1[4] * in[1] + m_centerToVertex1[5] * in[2];
	//out[2] = m_centerToVertex1[6] * in[0] + m_centerToVertex1[7] * in[1] + m_centerToVertex1[8] * in[2];
	//cv::Vec3d center = Compute3DLocalToWorld(NULL, frame, cv::Vec3d(0, 0, 0));
	//std::cout << " 源点  " << m_globalPt3D[1][0] << " " << m_globalPt3D[1][1] << " " << m_globalPt3D[1][2] << " \n";
	//std::cout << " 求的点  " << center[0] + out[0] << " " << center[1] + out[1] << " " << center[2] + out[2] << " \n";
}
