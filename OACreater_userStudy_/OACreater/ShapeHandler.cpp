#include "ShapeHandler.h"


CShapeHandler::CShapeHandler()
{
	m_activePartIdx = -1;
}

CShapeHandler::CShapeHandler(const CShapeHandler& shapeHandler)
{
	m_activePartIdx = shapeHandler.m_activePartIdx;
	m_allParts.clear();
	for (int i = 0; i < shapeHandler.m_allParts.size(); ++i)
	{
		CShapePart* newPart;
		newPart = new CShapePart(*(shapeHandler.m_allParts[i]));
		m_allParts.push_back(newPart);
	}
}


CShapeHandler::~CShapeHandler()
{
	DestoryAllParts();
}

void CShapeHandler::DestoryAllParts()
{
	m_allParts.clear();
}

void CShapeHandler::DrawAllParts(QColor modelColor)
{
	for (int i = 0; i < m_allParts.size(); ++i)
	{
		m_allParts[i]->DrawPart(modelColor);
	}
}

void CShapeHandler::DrawAllParts()
{
	for (int i = 0; i < m_allParts.size(); ++i)
	{
		m_allParts[i]->DrawPart();
	}
}

void CShapeHandler::DrawAllParts(qglviewer::Camera* c)
{
	for (int i = 0; i < m_allParts.size(); ++i)
	{
		m_allParts[i]->DrawPart(c);
	}
}

void CShapeHandler::DrawAllPartsWithNames()
{
	for (int i = 0; i < m_allParts.size(); ++i)
	{
		glPushName(i);
		m_allParts[i]->DrawPart();
		glPopName();
	}
}

void CShapeHandler::AddNewPart(CShapePart* newShapePart)
{
	m_allParts.push_back(newShapePart);
	m_activePartIdx = m_allParts.size() - 1;
}

void CShapeHandler::AddNewPart(CShapePart newShapePart)
{
	CShapePart* ShapePart = new CShapePart(newShapePart);
	m_allParts.push_back(ShapePart);
	m_activePartIdx = m_allParts.size() - 1;
}

CShapePart* CShapeHandler::GetActivePart()
{
	if (m_activePartIdx >= 0 && m_activePartIdx < m_allParts.size())
		return m_allParts[m_activePartIdx];
	else
		return NULL;
}

bool CShapeHandler::SetActivePart(int idx)
{
	if (idx >= 0 && idx < m_allParts.size())
	{
		m_activePartIdx = idx;
		return true;
	}
	else
	{
		m_activePartIdx = -1;
		return false;
	}
}

CShapeHandler& CShapeHandler::operator=(const CShapeHandler& shapeHandler)
{
	DestoryAllParts();
	m_activePartIdx = shapeHandler.m_activePartIdx;

	for (int i = 0; i < shapeHandler.m_allParts.size(); ++i)
	{
		CShapePart* newPart;
		newPart = new CShapePart(*(shapeHandler.m_allParts[i]));
		m_allParts.push_back(newPart);
	}
	return *this;
}

void CShapeHandler::SaveWholeModel(const char* filePath, std::vector<bool> floatings)
{
	std::fstream fout;
	fout.open(filePath, std::ios::out);
	std::string dataDir = filePath;
	size_t found = dataDir.find_last_of('.');
	dataDir = dataDir.substr(0, found);
	dataDir = dataDir + "_data";
	mkdir(dataDir.c_str());
	//相对路径
	std::string dataPath = "_data";
	fout << m_allParts.size() << std::endl;
	for (int i = 0; i < floatings.size();i++)
	{
		fout << floatings[i]<< "\t";
	}
	fout << std::endl;
	for (int i = 0; i < m_allParts.size(); ++i)
	{
		std::string thisModelPath;
		std::string thisViewPath;
		thisModelPath = dataDir + "/" + std::to_string(i) + ".obj";
		thisViewPath = dataDir + "/" + std::to_string(i) + ".vmat";
		//相对路径
		std::string modelPath = dataPath + "/" + std::to_string(i) + ".obj";
		std::string viewPath = dataPath + "/" + std::to_string(i) + ".vmat";
		m_allParts[i]->m_mesh->write(thisModelPath);
		CViewMat thisViewMat;
		thisViewMat = *(m_allParts[i]->m_frame);
		thisViewMat.SaveView(thisViewPath.c_str());
		fout << modelPath << "\t" << viewPath << std::endl;
	}
	std::string cameraViewPath = dataPath + "/" + "camera.vmat";
	fout << cameraViewPath << std::endl;
	fout.flush();
	fout.close();
	fout.clear();
}

void CShapeHandler::ComputeEdgeAndNormal(qglviewer::Camera* c)
{
	for (int i = 0; i < m_allParts.size(); i++)
	{
		m_allParts[i]->ComputeBoxEdgeAndNormal(c);
	}

}

void CShapeHandler::ComputeEdgeAndNormal(qglviewer::Camera* c, Edge e)
{
	if (e.m_shapeIndex > m_allParts.size() - 1)
		return;
	if (e.m_shapeIndex < 0)
		return;
	m_allParts[e.m_shapeIndex]->ComputeBoxEdgeAndNormal(c);
}

void CShapeHandler::ComputeEdgeAndNormal(qglviewer::Camera* c, int shapeIndex)
{
	if (shapeIndex > m_allParts.size() - 1)
		return;
	if (shapeIndex < 0)
		return;
	m_allParts[shapeIndex]->ComputeBoxEdgeAndNormal(c);
}

std::vector<std::vector<Edge>> CShapeHandler::GetEdgeRelationsWithRansac()
{
	std::vector<Edge> edges;
	for (int i = 0; i < m_allParts.size(); i++)
	{
		for (int j = 0; j < m_allParts[i]->edges.size(); j++)
		{
			m_allParts[i]->edges[j].m_shapeIndex = i;
			if (gdv_isFourRelation)
			{
				edges.push_back(m_allParts[i]->edges[j]);
			}
			else
			if (m_allParts[i]->edges[j].m_isVisible)
				edges.push_back(m_allParts[i]->edges[j]);
		}
	}
	for (int i = 0; i < edges.size();i++)
	{
		edges[i].m_allEdgeNo = i;
	}
	std::vector<std::vector<Edge>> sets;//关系集合，一个关系里面包括多条边
	if (edges.size() == 0)
		return sets;
	while (true)
	{
		int ransac_times;
		if (m_allParts.size() < 20)
			ransac_times = 50;//50 //设置合理的次数，不能消耗太多时间
		else
			ransac_times = 70;//70
		Edge line;
		int voteCnt = 0;//投票数
		int maxVoteCnt = 0;//最大投票数
		int initialValue = 0;//ransac算法初始值，消除ransac的随机性
		std::vector<Edge> get_edges;
		for (int i = 0; i < ransac_times; i++)
		{
			if (edges.size() == 0)
				return sets;
			int selectedIndex = initialValue % edges.size();//rand()
			initialValue++;
			line = edges[selectedIndex];//选择的线
			voteCnt = 0;//投票
			std::vector<Edge> subEdges;//候选关系
			subEdges.push_back(line);
			for (int k = 0; k < edges.size(); k++)
			{
				if (selectedIndex == k)
				{
					continue;//是选的边本身的话直接跳出
				}
				if (edges[selectedIndex].m_shapeIndex == edges[k].m_shapeIndex)
				{
					continue;//如果属于一个shape也跳出
				}
				float dis = ComputEdgeToLineDis(line, edges[k]);
				edges[k].m_edgeToLineDis = dis;//
				//if (dis == 0)
				//{
				//	continue;//边的方向差异比较大退出
				//}
				//std::cout << "dis is" << dis << "\n";
				//判断距离以及3D中的方向
				if (dis<gdv_unitThreshold && (abs(line.m_dir3D.dot(edges[k].m_dir3D))>0.95 || isnan(abs(line.m_dir3D.dot(edges[k].m_dir3D))))) //20 0.95
				{
					std::vector<Edge>::iterator it;
					int flag = 0;
					for (it = subEdges.begin(); it != subEdges.end(); it++)
					{
						if (it->m_shapeIndex == edges[k].m_shapeIndex)
						{
							flag = 1;
							//换一个距离更小的，一个shape智能有一条边
							if (it->m_edgeToLineDis > edges[k].m_edgeToLineDis)
							{
								it = subEdges.erase(it);
								subEdges.push_back(edges[k]);
								break;
							}
						}

					}
					if (flag == 0)
					{
						//edges[k].m_edgeToLineDis = dis;
						subEdges.push_back(edges[k]);
						voteCnt++;
					}
				}
			}
			if (voteCnt > maxVoteCnt)
			{
				maxVoteCnt = voteCnt;
				get_edges.clear();

				for (int i = 0; i < subEdges.size(); i++)
				{
					get_edges.push_back(subEdges[i]);
				}
			}

		}
		if (get_edges.size() < 1)
		{
			break;
		}
		sets.push_back(get_edges);
		for (int i = 0; i < get_edges.size(); i++)
		{
			std::vector<Edge>::iterator it;
			for (it = edges.begin(); it != edges.end(); it++)
			{
				if (get_edges[i].m_allEdgeNo == it->m_allEdgeNo)
				{
					edges.erase(it);
					break;
				}
			}
		}
		
	}
	return sets;

}

float CShapeHandler::ComputEdgeToLineDis(Edge line, Edge edge)
{
	//dir and distance
	float sum = 0;
	std::vector<cv::Vec2d> points = edge.m_points;
	if (abs(line.m_dir2D.dot(edge.m_dir2D)) > 0.95 || isnan(abs(line.m_dir2D.dot(edge.m_dir2D)))) //0.95
	{
		for (int i = 0; i < points.size(); i++)
		{
			sum += Utils::CalculatePointToLineDistance(line.m_dir2D, line.m_projectPt2D[0], points[i]);
		}

	}
	else
	{
	    return 1000;
	}
	if (points.size() == 0)
		return 1000;
	return sum / points.size();
}

void CShapeHandler::FixAllModels(qglviewer::Camera* c)
{
	for (int i = 0; i < m_allParts.size(); i++)
	{
		if (m_allParts[i]->m_isFloating)
		{
			m_allParts[i]->m_isFloating = false;
			//切换float/fix的时候保持位置不变
			GLdouble matrix[16];
			for (int k = 0; k < 16; k++)
			{
				matrix[k] = m_allParts[i]->m_frame->matrix()[k];
			}
			GLdouble camera_matrix[16];
			for (int k = 0; k < 16; k++)
			{
				camera_matrix[k] = c->frame()->matrix()[k];
			}
			Utils::MatrixMult(matrix, camera_matrix);
			//glMultMatrixd(camera_matrix);
			if (gdv_isRotateWithCamera)
			{
				//加入相机旋转矩阵
				camera_matrix[12] = 0;
				camera_matrix[13] = 0;
				camera_matrix[14] = 0;
				Utils::MatrixMult44(matrix, camera_matrix);
			}
			//
			m_allParts[i]->m_frame->setFromMatrix(matrix);
			// 设置参考frame为NULL，解除悬浮状态，使其具有位置
			m_allParts[i]->m_frame->setReferenceFrame(NULL);
		}
	}
}
