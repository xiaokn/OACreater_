#include "displaywidget.h"
CDisplayWidget::CDisplayWidget(QWidget *parent)
	: QGLViewer(QGLFormat(QGL::SampleBuffers), parent)
{
	InitializeData();
	InitializeWidget();
	InitializeMouseBinding();
	UpdateManipulatedFrame();
}


CDisplayWidget::~CDisplayWidget()
{
	DestoryData();
}

void CDisplayWidget::draw()
{
	Draw3DFuncs();
}

void CDisplayWidget::paintEvent(QPaintEvent* e)
{
	if (m_shapeHandler_h == NULL) return;
	m_shapePart = NULL;
	for (int i = 0; i < m_shapeHandler_h->m_allParts.size(); i++)
	{
		if (m_shapeHandler_h->m_allParts[i]->m_frame == manipulatedFrame())
		{
			m_shapePart = m_shapeHandler_h->m_allParts[i];
			break;
		}
	}
	if (m_isClearMouseBinding)
	{
		clearMouseBindings();
	}
	else
	{
		if (manipulatedFrame() == camera()->frame())
		{
			clearMouseBindings();
			//if (m_mouseOperateType==SC::OperateCamera)
			//{
			//	InitializeMouseBinding();
			//}
			//else
			//{
			//	setManipulatedFrame(camera()->frame());
			//	m_shapePart = NULL;
			//	InitializeMouseBinding();
			//	//clearMouseBindings();
			//	//setMouseBinding(Qt::NoModifier, Qt::LeftButton, QGLViewer::SELECT, true);
			//}
			setMouseBinding(Qt::NoModifier, Qt::LeftButton, QGLViewer::SELECT);
		}
		else
		{
			if (m_stopShapeRotation)
			{
				clearMouseBindings();
				//setMouseBinding(Qt::NoModifier, Qt::LeftButton, QGLViewer::FRAME, QGLViewer::ROTATE);
				setMouseBinding(Qt::NoModifier, Qt::RightButton, QGLViewer::FRAME, QGLViewer::TRANSLATE);
				//setWheelBinding(Qt::NoModifier, QGLViewer::FRAME, QGLViewer::ZOOM);
				//std::cout << "hahhahhahhahah\n";
				setMouseBinding(Qt::NoModifier, Qt::LeftButton, QGLViewer::SELECT);

			}
			else
			{
				InitializeMouseBinding();
			}
		}
	}
	if (m_mouseOperateType == SC::OperateCamera)
	{
		setManipulatedFrame(camera()->frame());
		m_shapePart = NULL;
		clearMouseBindings();
		/*setMouseBinding(Qt::AltModifier, Qt::LeftButton, QGLViewer::CAMERA, QGLViewer::ROTATE);
		setMouseBinding(Qt::AltModifier, Qt::RightButton, QGLViewer::CAMERA, QGLViewer::TRANSLATE);
		setMouseBinding(Qt::AltModifier, Qt::MidButton, QGLViewer::CAMERA, QGLViewer::ZOOM);
		setWheelBinding(Qt::AltModifier, QGLViewer::CAMERA, QGLViewer::ZOOM);*/
		setMouseBinding(Qt::NoModifier, Qt::RightButton, QGLViewer::FRAME, QGLViewer::ROTATE);
		setMouseBinding(Qt::NoModifier, Qt::LeftButton, QGLViewer::FRAME, QGLViewer::TRANSLATE);
		setWheelBinding(Qt::NoModifier, QGLViewer::FRAME, QGLViewer::ZOOM);
	}
	Q_UNUSED(e);
	QPainter painter;
	painter.begin(this);
	painter.setRenderHint(QPainter::Antialiasing);
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	// Reset OpenGL parameters
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	//glEnable(GL_LIGHTING);
	//glEnable(GL_LIGHT0);
	glEnable(GL_MULTISAMPLE);
	//glShadeModel(GL_FLAT);         // Set Smooth Shading                 
	glClearDepth(1.0f);                 // Depth buffer setup             
	glEnable(GL_DEPTH_TEST);         // Enables Depth Testing             
	glDepthFunc(GL_LEQUAL);             // The Type Of Depth Test To Do     
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);     /* Really Nice Perspective Calculations */

	glEnable(GL_CULL_FACE);
	//glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.0f, 1.0f);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glMatrixMode(GL_PROJECTION);

	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);

	setBackgroundColor(QColor(255, 255, 255));
	qglClearColor(backgroundColor());
	//QColor bgColor = backgroundColor();
	preDraw();
	draw();
	postDraw();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glPopAttrib();
	m_shapeHandler_h->ComputeEdgeAndNormal(this->camera());//计算边和法线
	if (m_shapePart != NULL&&!m_shapePart->edges.empty())
		m_shapePart->DrawPart(painter);
	if(m_isShowNo)
	DrawShapeNo(painter);
	DrawOtherFunction(painter);
	if (!m_stopRelationUpdate)
		DrawRelitions(painter);//关系的可视化
	if (m_isShowSpacingRelations)
	{
		DrawSpacings(painter);
	}
	else
	{
		m_SpacingRelations.clear();
		m_addSpacingRelations.clear();
		m_addSpacingRelationsGroup.clear();
		m_deleteSpacingRelations.clear();
	}
	/*if (m_mouseOperateType==SC::DeleteRelations||m_mouseOperateType==SC::AddRelations)
	{
	DrawEditRelitions(painter);
	}
	else
	{
	m_editRelations.clear();
	}*/
	painter.end();
}

void CDisplayWidget::paintGL()
{
	update();
}

QString CDisplayWidget::helpString() const
{
	QString text("<h2>OACreater</h2>");
	return text;
}

void CDisplayWidget::keyPressEvent(QKeyEvent* e)
{
	QGLViewer::keyPressEvent(e);
}

void CDisplayWidget::mousePressEvent(QMouseEvent* e)
{
	m_startPoint[0] = e->x();
	m_startPoint[1] = e->y();
	emit BackupDataSender();
	switch (m_mouseOperateType)
	{
	case SC::RecSelect:
	{
		m_stopRelationUpdate = false;
		m_selectShapePoint[0][0] = e->x();
		m_selectShapePoint[0][1] = e->y();
		m_selectShapePoint[1][0] = e->x();
		m_selectShapePoint[1][1] = e->y();
		m_partShapeHandler_h.DestoryAllParts();
		break;
	}
	case SC::CurveAlignment:
	{
		m_curveStrokeAL.clear();
		m_curveStrokeAL.push_back(m_startPoint);
		break;
	}
	case SC::AddRelations:
	{
		m_relationStrokeAL.clear();
		m_relationStrokeAL.push_back(m_startPoint);
		if (m_writerp != NULL&&m_mouseOperateType == SC::AddRelations)
		{
			(*m_writerp) << "EDIT_RELATION_BEGIN " << (double)clock() << std::endl;
			m_writerp->flush();
			m_isEditRelaion = true;
		}
		break;
	}
	case SC::DeleteRelations:
	{
		m_relationStrokeAL.clear();
		m_relationStrokeAL.push_back(m_startPoint);
		break;
	}
	case SC::OperateShape:
	{
		if (m_shapePart != NULL)
		{
			if (m_isZoom)
			{
				bool isChange = false;
				if (m_shapePart->m_isFloating)
				{
					isChange = true;
					m_shapePart->m_isFloating = false;
					//切换float/fix的时候保持位置不变
					GLdouble matrix[16];
					for (int k = 0; k < 16; k++)
					{
						matrix[k] = m_shapePart->m_frame->matrix()[k];
					}
					GLdouble camera_matrix[16];
					for (int k = 0; k < 16; k++)
					{
						camera_matrix[k] = camera()->frame()->matrix()[k];
					}
					Utils::MatrixMult(matrix, camera_matrix);
					//glMultMatrixd(camera_matrix);
					m_shapePart->m_frame->setFromMatrix(matrix);
					// 设置参考frame为NULL，解除悬浮状态，使其具有位置
					m_shapePart->m_frame->setReferenceFrame(NULL);
				}
				int i = 0;
				for (i = 0; i < m_selectRect.size(); i++)
				{
					if (m_selectRect[i].contains(m_startPoint[0], m_startPoint[1]))
					{
						m_scaleStartPoint[0] = 0;
						m_scaleStartPoint[1] = 0;
						m_scaleStartPoint[2] = 0;
						m_scaleStartPoint = m_recordCenterPoints[i];
						setCursor(Qt::CrossCursor);
						m_scaleCount = 0;
						m_recordFrontProPoint = cv::Vec3d(0, 0, 0);
						break;
					}
				}
				if (i == m_selectRect.size())
				{
					if (m_isClearMouseBinding)
					{
						clearMouseBindings();
					}
					else
					{
						if (m_stopShapeRotation)
						{
							clearMouseBindings();
							//setMouseBinding(Qt::NoModifier, Qt::LeftButton, QGLViewer::FRAME, QGLViewer::ROTATE);
							//setMouseBinding(Qt::NoModifier, Qt::RightButton, QGLViewer::FRAME, QGLViewer::TRANSLATE);
							setWheelBinding(Qt::NoModifier, QGLViewer::FRAME, QGLViewer::ZOOM);
							setMouseBinding(Qt::NoModifier, Qt::LeftButton, QGLViewer::SELECT);
							QGLViewer::mousePressEvent(e);
							setMouseBinding(Qt::NoModifier, Qt::LeftButton, QGLViewer::FRAME, QGLViewer::TRANSLATE);
						
						}
						else
						{
							clearMouseBindings();
							setMouseBinding(Qt::NoModifier, Qt::LeftButton, QGLViewer::SELECT);
							QGLViewer::mousePressEvent(e);
							setMouseBinding(Qt::NoModifier, Qt::RightButton, QGLViewer::FRAME, QGLViewer::ROTATE);
							setMouseBinding(Qt::NoModifier, Qt::LeftButton, QGLViewer::FRAME, QGLViewer::TRANSLATE);
		
						}
					}
				}
				else
				{
					clearMouseBindings();
				}
				//复原
				if (isChange)
				{
					m_shapePart->m_isFloating = true;
					//切换float/fix的时候保持位置不变
					GLdouble camera_matrix[16];
					cv::Mat mat_m = cv::Mat::zeros(4, 4, CV_64F);
					for (int m = 0; m < 4; m++)
					{
						for (int n = 0; n < 4; n++)
						{
							mat_m.at<double>(m, n) = camera()->frame()->matrix()[m * 4 + n];
						}
					}
					mat_m = mat_m.inv();
					for (int m = 0; m < 4; m++)
					{
						for (int n = 0; n < 4; n++)
						{
							camera_matrix[m * 4 + n] = mat_m.at<double>(m, n);
						}
					}
					GLdouble matrix[16];
					for (int k = 0; k < 16; k++)
					{
						matrix[k] = m_shapePart->m_frame->matrix()[k];
					}
					Utils::MatrixMult(matrix, camera_matrix);
					//glMultMatrixd(camera_matrix);
					m_shapePart->m_frame->setFromMatrix(matrix);
					//设置参考frame为相机，使位置处于悬浮状态
					m_shapePart->m_frame->setReferenceFrame(camera()->frame());
				}
			}
			else
			{
				if (m_stopShapeRotation)
				{
					clearMouseBindings();
					//setMouseBinding(Qt::NoModifier, Qt::LeftButton, QGLViewer::FRAME, QGLViewer::ROTATE);
					//setMouseBinding(Qt::NoModifier, Qt::RightButton, QGLViewer::FRAME, QGLViewer::TRANSLATE);
					setWheelBinding(Qt::NoModifier, QGLViewer::FRAME, QGLViewer::ZOOM);
					setMouseBinding(Qt::NoModifier, Qt::LeftButton, QGLViewer::SELECT);
					QGLViewer::mousePressEvent(e);
					setMouseBinding(Qt::NoModifier, Qt::LeftButton, QGLViewer::FRAME, QGLViewer::TRANSLATE);

				}
				else
				{
					clearMouseBindings();
					setMouseBinding(Qt::NoModifier, Qt::LeftButton, QGLViewer::SELECT);
					QGLViewer::mousePressEvent(e);
					setMouseBinding(Qt::NoModifier, Qt::RightButton, QGLViewer::FRAME, QGLViewer::ROTATE);
					setMouseBinding(Qt::NoModifier, Qt::LeftButton, QGLViewer::FRAME, QGLViewer::TRANSLATE);

				}
			}
			/*if (m_writerp != NULL)
			{
			(*m_writerp) << "MOVE_SHAPE_BEGIN " << (double)clock() << std::endl;
			m_writerp->flush();
			m_isMoveShape = true;
			}*/
		}
		else
		{
			if (m_stopShapeRotation)
			{
				clearMouseBindings();
				setWheelBinding(Qt::NoModifier, QGLViewer::FRAME, QGLViewer::ZOOM);
				setMouseBinding(Qt::NoModifier, Qt::LeftButton, QGLViewer::SELECT);
				QGLViewer::mousePressEvent(e);
				setMouseBinding(Qt::NoModifier, Qt::LeftButton, QGLViewer::FRAME, QGLViewer::TRANSLATE);

			}
			else
			{
				clearMouseBindings();
				setMouseBinding(Qt::NoModifier, Qt::LeftButton, QGLViewer::SELECT);
				QGLViewer::mousePressEvent(e);
				setMouseBinding(Qt::NoModifier, Qt::RightButton, QGLViewer::FRAME, QGLViewer::ROTATE);
				setMouseBinding(Qt::NoModifier, Qt::LeftButton, QGLViewer::FRAME, QGLViewer::TRANSLATE);

			}
		}
		if (manipulatedFrame() == camera()->frame())
		{
			clearMouseBindings();
		}
		for (int i = 0; i < m_shapeHandler_h->m_allParts.size(); i++)
		{
			if (m_shapeHandler_h->m_allParts[i]->m_frame == manipulatedFrame())
			{
				m_shapePart = m_shapeHandler_h->m_allParts[i];
				break;
			}
		}
		if (m_writerp != NULL&&m_shapePart!=NULL)
		{
			(*m_writerp) << "MOVE_SHAPE_BEGIN " << (double)clock() << std::endl;
			m_writerp->flush();
			m_isMoveShape = true;
		}
		break;
	}
	case SC::AddSpacingStart:
	{
		m_spacingStrokeAL.clear();
		m_spacingStrokeAL.push_back(m_startPoint);
		if (m_writerp != NULL&&m_mouseOperateType == SC::AddSpacingStart)
		{
			(*m_writerp) << "ADD_SPACING_BEGIN " << (double)clock() << std::endl;
			m_writerp->flush();
		}
		break;
	}
	case SC::RemoveSpacing:
	{
		m_spacingStrokeAL.clear();
		m_spacingStrokeAL.push_back(m_startPoint);
		if (m_writerp != NULL&&m_mouseOperateType == SC::RemoveSpacing)
		{
			(*m_writerp) << "REMOVE_SPACING_BEGIN " << (double)clock() << std::endl;
			m_writerp->flush();
		}
		break;
	}
	//case SC::OperateShape:break;
	}
	QGLViewer::mousePressEvent(e);
}

void CDisplayWidget::mouseDoubleClickEvent(QMouseEvent* e)
{
	QGLViewer::mouseDoubleClickEvent(e);
}

void CDisplayWidget::mouseMoveEvent(QMouseEvent* e)
{
	QGLViewer::mouseMoveEvent(e);
	cv::Vec2d m_startPt;
	m_startPt[0] = e->x();
	m_startPt[1] = e->y();
	switch (m_mouseOperateType)
	{
	case SC::RecSelect:
	{
		m_selectShapePoint[1][0] = e->x();
		m_selectShapePoint[1][1] = e->y();
		break;
	}
	case SC::CurveAlignment:
	{
		m_curveStrokeAL.push_back(m_startPt);
		break;
	}
	case SC::AddRelations:
	{
		m_relationStrokeAL.push_back(m_startPt);
		break;
	}
	case SC::DeleteRelations:
	{
		m_relationStrokeAL.push_back(m_startPt);
		break;
	}
	case SC::OperateShape:
	{
	//	m_SpacingRelations.clear();
		if (cursor().shape() == Qt::CrossCursor&&m_isZoom)
		{
			if (m_shapePart != NULL)
			{
				bool isChange = false;
				if (m_shapePart->m_isFloating)
				{
					isChange = true;
					m_shapePart->m_isFloating = false;
					//切换float/fix的时候保持位置不变
					GLdouble matrix[16];
					for (int k = 0; k < 16; k++)
					{
						matrix[k] = m_shapePart->m_frame->matrix()[k];
					}
					GLdouble camera_matrix[16];
					for (int k = 0; k < 16; k++)
					{
						camera_matrix[k] = camera()->frame()->matrix()[k];
					}
					Utils::MatrixMult(matrix, camera_matrix);
					//glMultMatrixd(camera_matrix);
					m_shapePart->m_frame->setFromMatrix(matrix);
					// 设置参考frame为NULL，解除悬浮状态，使其具有位置
					m_shapePart->m_frame->setReferenceFrame(NULL);
				}
				cv::Vec2d endPoint(e->x(), e->y());
				//m_shapePart
				ScaleShape(m_shapePart, m_scaleStartPoint, endPoint);
				//复原
				if (isChange)
				{
					m_shapePart->m_isFloating = true;
					//切换float/fix的时候保持位置不变
					GLdouble camera_matrix[16];
					cv::Mat mat_m = cv::Mat::zeros(4, 4, CV_64F);
					for (int m = 0; m < 4; m++)
					{
						for (int n = 0; n < 4; n++)
						{
							mat_m.at<double>(m, n) = camera()->frame()->matrix()[m * 4 + n];
						}
					}
					mat_m = mat_m.inv();
					for (int m = 0; m < 4; m++)
					{
						for (int n = 0; n < 4; n++)
						{
							camera_matrix[m * 4 + n] = mat_m.at<double>(m, n);
						}
					}
					GLdouble matrix[16];
					for (int k = 0; k < 16; k++)
					{
						matrix[k] = m_shapePart->m_frame->matrix()[k];
					}
					Utils::MatrixMult(matrix, camera_matrix);
					//glMultMatrixd(camera_matrix);
					m_shapePart->m_frame->setFromMatrix(matrix);
					//设置参考frame为相机，使位置处于悬浮状态
					m_shapePart->m_frame->setReferenceFrame(camera()->frame());
				}
			}
		}
		break;
	}
	case SC::AddSpacingStart:
	{
		m_spacingStrokeAL.push_back(m_startPt);
		break;
	}
	case SC::RemoveSpacing:
	{
		m_spacingStrokeAL.push_back(m_startPt);
		break;
	}
	//case SC::OperateShape:break;
	}
	repaint();
}

void CDisplayWidget::mouseReleaseEvent(QMouseEvent* e)
{
	QGLViewer::mouseReleaseEvent(e);
	m_endPoint[0] =e->x();
	m_endPoint[1] = e->y();
	switch (m_mouseOperateType)
	{
		case SC::RecSelect:
		{
			EditSelectShape();
			break;
		}
		case SC::CurveAlignment:
		{
			m_curveStrokeAL.push_back(m_endPoint);
			CloseToCurve();
			m_curveStrokeAL.clear();
			break;
		}
		case SC::AddRelations:
		{
			/*m_relationStrokeAL.push_back(m_endPoint);
			AddRelation();
			m_relationStrokeAL.clear();*/
			break;
		}
		/*case SC::DeleteRelations:
		{
			m_relationStrokeAL.push_back(m_endPoint);
			DeleteRelation();
			m_relationStrokeAL.clear();
			break;
		}*/
		case SC::OperateShape:
		{
			if (m_isZoom)
			{
				unsetCursor();
			}
			break;
		}
		case SC::AddSpacingStart:
		{
			AddSpacingRelations();
			m_spacingStrokeAL.clear();
			if (m_writerp != NULL&&m_mouseOperateType == SC::AddSpacingStart)
			{
				(*m_writerp) << "ADD_SPACING_END " << (double)clock() << std::endl;
				m_writerp->flush();
			}
			break;
		}
		case SC::RemoveSpacing:
		{
			RemoveSpacingRelations();
			m_spacingStrokeAL.clear();
			if (m_writerp != NULL&&m_mouseOperateType == SC::RemoveSpacing)
			{
				(*m_writerp) << "REMOVE_SPACING_END " << (double)clock() << std::endl;
				m_writerp->flush();
			}
			break;
		}
	//case SC::OperateShape:break;
	}
	if (!m_stopRelationUpdate)
	{
		UpdateRelations();
		m_relationStrokeAL.clear();
	}
	else
	{
		m_recordRelations.clear();
		m_optRelations.clear();
		m_relationStrokeAL.clear();
	}
	if (m_mouseOperateType == SC::AddRelations&&m_isEditRelaion)
	{
		if (m_writerp != NULL)
		{  
			if (m_editRelationType==1)
			{
				(*m_writerp) << "ADD_RELATION_END " << (double)clock() << std::endl;
			}
			else if (m_editRelationType == 2)
			{
				(*m_writerp) << "REMOVE_RELATION_END " << (double)clock() << std::endl;
			}
			else
			{
				(*m_writerp) << "NULL_EDIT_RELATION_END " << (double)clock() << std::endl;
			}
			m_writerp->flush();
		}
	}
	m_isEditRelaion = false;
	if (m_mouseOperateType == SC::OperateShape&&m_isMoveShape)
	{
		if (m_writerp != NULL)
		{
			(*m_writerp) << "MOVE_SHAPE_END " << (double)clock() << std::endl;
			m_writerp->flush();
		}
	}
	m_isMoveShape = false;
}

void CDisplayWidget::wheelEvent(QWheelEvent* e)
{
	QGLViewer::wheelEvent(e);
	/*if (m_shapePart!=NULL)
	{
	QPoint numPixels = e->pixelDelta();
	QPoint numDegrees = e->angleDelta() / 8;
	if (numDegrees.y() > 0)
	{
	trimesh::scale(m_shapePart->m_mesh, 1.03);
	}
	else
	{
	trimesh::scale(m_shapePart->m_mesh, 1 / 1.03);

	}
	}*/
	repaint();
}

void CDisplayWidget::resizeEvent(QResizeEvent* e)
{
	QGLViewer::resizeEvent(e);
}

void CDisplayWidget::drawWithNames()
{
	m_shapeHandler_h->DrawAllPartsWithNames();
}

void CDisplayWidget::postSelection(const QPoint& point)
{
	int selectedPartIdx = selectedName();
	m_shapeHandler_h->SetActivePart(selectedPartIdx);
	UpdateManipulatedFrame();
}

void CDisplayWidget::Draw3DFuncs()
{
	if(m_isShowGrid)
	DrawGrid(2, 3);//4 10  1.5 4 2 5
	//drawGrid(2,20);
	m_shapeHandler_h->DrawAllParts();
}

void CDisplayWidget::InitializeMouseBinding()
{
	clearMouseBindings();
	setMouseBinding(Qt::AltModifier, Qt::LeftButton, QGLViewer::CAMERA, QGLViewer::ROTATE);
	setMouseBinding(Qt::AltModifier, Qt::RightButton, QGLViewer::CAMERA, QGLViewer::TRANSLATE);
	setMouseBinding(Qt::AltModifier, Qt::MidButton, QGLViewer::CAMERA, QGLViewer::ZOOM);
	setWheelBinding(Qt::AltModifier, QGLViewer::CAMERA, QGLViewer::ZOOM);
	setMouseBinding(Qt::NoModifier, Qt::LeftButton, QGLViewer::FRAME, QGLViewer::ROTATE);
	setMouseBinding(Qt::NoModifier, Qt::RightButton, QGLViewer::FRAME, QGLViewer::TRANSLATE);
	setWheelBinding(Qt::NoModifier, QGLViewer::FRAME, QGLViewer::ZOOM);
	setMouseBinding(Qt::NoModifier, Qt::LeftButton, QGLViewer::SELECT,true);
}

void CDisplayWidget::UpdateManipulatedFrame()
{
	if (m_shapeHandler_h == NULL)
	{
		qglviewer::ManipulatedCameraFrame* cameraFrame = camera()->frame();
		setManipulatedFrame(cameraFrame);
		setWheelBinding(Qt::NoModifier, QGLViewer::FRAME, QGLViewer::ZOOM);
		return;
	}
	CShapePart* activePart = m_shapeHandler_h->GetActivePart();
	if (activePart != NULL)
	{
		setManipulatedFrame(activePart->m_frame);
		setWheelBinding(Qt::NoModifier, QGLViewer::FRAME, QGLViewer::NO_MOUSE_ACTION);
	}
	else
	{
		qglviewer::ManipulatedCameraFrame* cameraFrame = camera()->frame();
		setManipulatedFrame(cameraFrame);
		setWheelBinding(Qt::NoModifier, QGLViewer::FRAME, QGLViewer::ZOOM);
		//setMouseBinding(Qt::NoModifier, Qt::MidButton, QGLViewer::CAMERA, QGLViewer::ZOOM);
	}
}

void CDisplayWidget::InitializeData()
{

	m_shapeHandler_h = NULL;
	m_stopRelationUpdate = false;
	m_operateShape = false;
	m_isShowNo = true;
	m_stopShapeRotation = false;
	m_isClearMouseBinding = false;
	m_writerp = NULL;
	m_isMoveShape=false;
	m_isEditRelaion=false;
	m_editRelationType = -1;
	m_isZoom = true;
	m_isShowSpacingRelations = true;
	m_isShowGrid = true;
}

void CDisplayWidget::InitializeWidget()
{
	setManipulatedFrame(camera()->frame());
	camera()->frame()->setSpinningSensitivity(1000.0);
	camera()->setSceneRadius(camera()->sceneRadius() * 15);
	camera()->setPosition(camera()->position() * 3);
	camera()->setType(qglviewer::Camera::ORTHOGRAPHIC);
	setStateFileName(QString::null);
	camera()->frame()->setZoomSensitivity(2);
}

void CDisplayWidget::DestoryData()
{
	if (m_shapeHandler_h != NULL) m_shapeHandler_h=NULL;
}

void CDisplayWidget::SetData(CShapeHandler* shapeHandlerp)
{
	m_shapeHandler_h = shapeHandlerp;
}

void CDisplayWidget::DrawOtherFunction(QPainter& painter)
{
	switch (m_mouseOperateType)
	{
	case SC::RecSelect:
	{
		DrawSelectRect(painter);
		break;
	}
	case SC::CurveAlignment:
	{
		DrawCloseCurveStroke(painter);
		break;
	}
	case SC::AddRelations:
	{
		DrawEditRelationsStroke(painter);
		break;
	}
	case SC::DeleteRelations:
	{
		DrawEditRelationsStroke(painter);
		break;
	}
	case SC::OperateShape:
	{	
		if (m_isZoom)
		{
			DrawScaleFaceCenterPoint(painter);
		}
		break; 
	}
	case SC::AddSpacingStart:
	{
		DrawSpacingStroke(painter);
		break;
	}
	case SC::RemoveSpacing:
	{
		DrawSpacingStroke(painter);
		break;
	}
	//case SC::OperateShape:break;
	}
}

void CDisplayWidget::DrawScaleFaceCenterPoint(QPainter& painter)
{
	painter.setRenderHint(QPainter::Antialiasing, true);
	// 设置画笔颜色、宽度
	painter.setPen(QPen(QColor(0, 0, 0), 2));
	// 设置画刷颜色
	painter.setBrush(QColor(200, 200, 200));
	painter.save();
	if (m_shapePart!=NULL)
	{
		bool isChange = false;
		if (m_shapePart->m_isFloating)
		{
			isChange = true;
			m_shapePart->m_isFloating = false;
			//切换float/fix的时候保持位置不变
			GLdouble matrix[16];
			for (int k = 0; k < 16; k++)
			{
				matrix[k] = m_shapePart->m_frame->matrix()[k];
			}
			GLdouble camera_matrix[16];
			for (int k = 0; k < 16; k++)
			{
				camera_matrix[k] = camera()->frame()->matrix()[k];
			}
			Utils::MatrixMult(matrix, camera_matrix);
			//glMultMatrixd(camera_matrix);
			m_shapePart->m_frame->setFromMatrix(matrix);
			// 设置参考frame为NULL，解除悬浮状态，使其具有位置
			m_shapePart->m_frame->setReferenceFrame(NULL);
		}
		m_selectRect.clear();
		m_recordCenterPoints.clear();
		for (int j = 0; j < 6;j++)
		{
			Face face = m_shapePart->bboxFaces[j];
			bool b1, b2, b3, b4;
			b1 = m_shapePart->m_sharpConer[face.vertexIndex[0]].m_lbv_isVisible;
			b2 = m_shapePart->m_sharpConer[face.vertexIndex[1]].m_lbv_isVisible;
			b3 = m_shapePart->m_sharpConer[face.vertexIndex[2]].m_lbv_isVisible;
			b4 = m_shapePart->m_sharpConer[face.vertexIndex[3]].m_lbv_isVisible;
			if (b1&&b2&&b3&&b4)
			{
				painter.drawEllipse(QPoint(face.centerPoint[0], face.centerPoint[1]), 5, 5);//画圆			
				m_recordCenterPoints.push_back(face.center3DPoint);
				double x = face.centerPoint[0] - 5;
				double y = face.centerPoint[1] - 5;
				QRect rect(x, y, 10, 10);
				m_selectRect.push_back(rect);
			}
		}
		//复原
		if (isChange)
		{
			m_shapePart->m_isFloating = true;
			//切换float/fix的时候保持位置不变
			GLdouble camera_matrix[16];
			cv::Mat mat_m = cv::Mat::zeros(4, 4, CV_64F);
			for (int m = 0; m < 4; m++)
			{
				for (int n = 0; n < 4; n++)
				{
					mat_m.at<double>(m, n) = camera()->frame()->matrix()[m * 4 + n];
				}
			}
			mat_m = mat_m.inv();
			for (int m = 0; m < 4; m++)
			{
				for (int n = 0; n < 4; n++)
				{
					camera_matrix[m * 4 + n] = mat_m.at<double>(m, n);
				}
			}
			GLdouble matrix[16];
			for (int k = 0; k < 16; k++)
			{
				matrix[k] = m_shapePart->m_frame->matrix()[k];
			}
			Utils::MatrixMult(matrix, camera_matrix);
			//glMultMatrixd(camera_matrix);
			m_shapePart->m_frame->setFromMatrix(matrix);
			//设置参考frame为相机，使位置处于悬浮状态
			m_shapePart->m_frame->setReferenceFrame(camera()->frame());
		}
	}
	painter.restore();
}

void CDisplayWidget::ScaleShape(CShapePart *shapePart, cv::Vec3d startPoint, cv::Vec2d endPoint)
{
	cv::Vec3d axisX, axisY, axisZ, axis;
	axisX = cv::Vec3d(1, 0, 0);
	axisY = cv::Vec3d(0, 1, 0);
	axisZ = cv::Vec3d(0, 0, 1);
	//2d点
	cv::Vec2d startProPoint, centerProPoint;
	centerProPoint = ComputeProjectedVertex(shapePart->m_frame, cv::Vec3d(0, 0, 0));
	qglviewer::Vec localStartPoint = shapePart->m_frame->localCoordinatesOf(qglviewer::Vec(startPoint[0], startPoint[1], startPoint[2])); //世界坐标系转地方坐标系
	startProPoint = ComputeProjectedVertex(shapePart->m_frame, cv::Vec3d(localStartPoint[0], localStartPoint[1], localStartPoint[2]));
	//cv::Vec2d dir2D = startProPoint - centerProPoint;
	cv::Vec2d objectProPoint = Utils::pointToLineProjection(centerProPoint, startProPoint, endPoint);
	qglviewer::Vec localObjectPoint, globalObjectPoint;
	//对齐
	globalObjectPoint.x = objectProPoint[0];
	globalObjectPoint.y = objectProPoint[1];
	globalObjectPoint.z = 0;	//近平面的点
	qglviewer::Vec nearPos = camera()->unprojectedCoordinatesOf(globalObjectPoint);
	globalObjectPoint.z = 1;	//远平面的点
	qglviewer::Vec farPos = camera()->unprojectedCoordinatesOf(globalObjectPoint);
	cv::Vec3d nearPoint, farPoint, centerPoint, faceCenterPoint;
	nearPoint = cv::Vec3d(nearPos[0], nearPos[1], nearPos[2]);
	farPoint = cv::Vec3d(farPos[0], farPos[1], farPos[2]);
	qglviewer::Vec p = shapePart->m_frame->inverseCoordinatesOf(qglviewer::Vec(0, 0, 0));
	centerPoint = cv::Vec3d(p[0], p[1], p[2]);
	faceCenterPoint = startPoint;
	cv::Vec3d objectPoint = Utils::CalculateCrossPoint(nearPoint, farPoint, centerPoint, faceCenterPoint);
	if (objectPoint == cv::Vec3d(0, 0, 0))
		return;
	qglviewer::Vec localF, localO;
	localF = shapePart->m_frame->localCoordinatesOf(qglviewer::Vec(faceCenterPoint[0], faceCenterPoint[1], faceCenterPoint[2]));
	localO = shapePart->m_frame->localCoordinatesOf(qglviewer::Vec(objectPoint[0], objectPoint[1], objectPoint[2]));
	cv::Vec3d localFaceCenterPoint(localF[0], localF[1], localF[2]);
	cv::Vec3d locObjectPoint(localO[0], localO[1], localO[2]);
	//
	if (m_scaleCount == 0)
	{
		m_recordFrontProPoint = locObjectPoint;
	}
	else
	{
		localFaceCenterPoint = m_recordFrontProPoint;
		m_recordFrontProPoint = locObjectPoint;
	}
	m_scaleCount++;
	double scale = Utils::Calculate2DTwoPointDistance(cv::Vec3d(0, 0, 0), locObjectPoint) / Utils::Calculate2DTwoPointDistance(cv::Vec3d(0, 0, 0), localFaceCenterPoint);
	double dis = abs(localFaceCenterPoint.dot(axisX));
	axis = axisX;
	if (abs(localFaceCenterPoint.dot(axisY)) > dis)
	{
		dis = abs(localFaceCenterPoint.dot(axisY));
		axis = axisY;
	}
	if (abs(localFaceCenterPoint.dot(axisZ)) > dis)
	{
		dis = abs(localFaceCenterPoint.dot(axisZ));
		axis = axisZ;
	}
	if (scale < 0 || scale == 0)
		return;
	//缩放
	if (axis[0] != 0)
	{
		trimesh::scale(m_shapePart->m_mesh, scale, 1, 1);
	}
	else if (axis[1] != 0)
	{
		trimesh::scale(m_shapePart->m_mesh, 1, scale, 1);
	}
	else if (axis[2] != 0)
	{
		trimesh::scale(m_shapePart->m_mesh, 1, 1, scale);
	}
}

cv::Vec2d CDisplayWidget::ComputeProjectedVertex(qglviewer::ManipulatedFrame* frame, cv::Vec3d Pt3D)
{
	qglviewer::Vec globalPos, localPos, projectedPos;
	localPos.x = Pt3D[0];
	localPos.y = Pt3D[1];
	localPos.z = Pt3D[2];
	globalPos = frame->inverseCoordinatesOf(localPos);
	projectedPos = camera()->projectedCoordinatesOf(globalPos);
	cv::Vec2d thisPt;
	thisPt[0] = projectedPos.x;
	thisPt[1] = projectedPos.y;
	//returnM(c, m);
	return thisPt;
}

void CDisplayWidget::CloseToCurve()
{
	if (m_curveStrokeAL.size() == 0)
		return;
	for (int i = 0; i < m_shapeHandler_h->m_allParts.size(); i++)
	{
		cv::Vec2d center2DPoint = m_shapeHandler_h->m_allParts[i]->m_centerPointProjected;
		cv::Vec3d center3DPoint = m_shapeHandler_h->m_allParts[i]->m_centerPoint;
		CShapePart* activePart = m_shapeHandler_h->m_allParts[i];
		double dis;
		cv::Vec2d alignPos;
		for (int j = 0; j < m_curveStrokeAL.size(); j++)
		{
			cv::Vec2d objPoint;
			objPoint[0] = m_curveStrokeAL[j][0];
			objPoint[1] = m_curveStrokeAL[j][1];
			if (j == 0)
			{
				dis = Utils::Calculate2DTwoPointDistance(center2DPoint, objPoint);
				alignPos = objPoint;
			}
			else
			{
				if (dis > Utils::Calculate2DTwoPointDistance(center2DPoint, objPoint))
				{
					dis = Utils::Calculate2DTwoPointDistance(center2DPoint, objPoint);
					alignPos = objPoint;
				}
			}
		}
		//对齐
		qglviewer::Vec vec;
		vec.x = alignPos[0];
		vec.y = alignPos[1];
		vec.z = 0;	//近平面的点
		qglviewer::Vec nearPos = camera()->unprojectedCoordinatesOf(vec);
		vec.z = 1;	//远平面的点
		qglviewer::Vec farPos = camera()->unprojectedCoordinatesOf(vec);
		//计算投影点
		cv::Vec3d nearPoint, farPoint;
		nearPoint[0] = nearPos[0]; nearPoint[1] = nearPos[1]; nearPoint[2] = nearPos[2];
		farPoint[0] = farPos[0]; farPoint[1] = farPos[1]; farPoint[2] = farPos[2];
		cv::Vec3d pos = Utils::ClosestPointOnSegment(center3DPoint, nearPoint, farPoint);
		//让shape移动到pos
		activePart->m_frame->setPosition(qglviewer::Vec(pos[0], pos[1], pos[2]));

	}
}

void CDisplayWidget::DrawCloseCurveStroke(QPainter& painter)
{
	if (m_curveStrokeAL.size() == 0)
		return;
	painter.save();
	painter.setRenderHint(QPainter::Antialiasing, true);
	QColor c(200, 0, 0, 200);
	QPen thisPen(QBrush(c), 5, Qt::SolidLine, Qt::RoundCap);
	thisPen.setJoinStyle(Qt::RoundJoin);
	painter.setPen(thisPen);
	QPainterPath path;
	path.moveTo(m_curveStrokeAL[0][0], m_curveStrokeAL[0][1]);
	for (int i = 1; i < m_curveStrokeAL.size(); ++i)
	{
		path.lineTo(m_curveStrokeAL[i][0], m_curveStrokeAL[i][1]);
		path.moveTo(m_curveStrokeAL[i][0], m_curveStrokeAL[i][1]);
	}
	painter.drawPath(path);
	painter.restore();
}

void CDisplayWidget::EditSelectShape()
{
	double startX = m_selectShapePoint[0][0];
	double startY = m_selectShapePoint[0][1];
	double endX = m_selectShapePoint[1][0];
	double endY = m_selectShapePoint[1][1];
	m_partShapeHandler_h.DestoryAllParts();
	//根据画的矩形选中矩形内部的shape
	for (int i = 0; i < m_shapeHandler_h->m_allParts.size(); i++)
	{
		double x = m_shapeHandler_h->m_allParts[i]->m_centerPointProjected[0];
		double y = m_shapeHandler_h->m_allParts[i]->m_centerPointProjected[1];
		if (x > startX&&y > startY&&x < endX&&y < endY)
		{
			m_partShapeHandler_h.m_allParts.push_back(m_shapeHandler_h->m_allParts[i]);
		}
	}
	//清空选中的shape
	m_selectShapePoint[0][0] = 0;
	m_selectShapePoint[0][1] = 0;
	m_selectShapePoint[1][0] = 0;
	m_selectShapePoint[1][1] = 0;
}

void CDisplayWidget::DrawSelectRect(QPainter& painter)
{
	QPen thisPen;
	thisPen.setBrush(Qt::black);
	thisPen.setWidth(1);

	QRect selectRect(m_selectShapePoint[0][0], m_selectShapePoint[0][1], m_selectShapePoint[1][0] - m_selectShapePoint[0][0], m_selectShapePoint[1][1] - m_selectShapePoint[0][1]);
	painter.save();
	painter.setPen(thisPen);

	painter.setBrush(QColor(100, 100, 100, 100));
	painter.drawRect(selectRect);
	painter.restore();
	//
	painter.setPen(QPen(Qt::blue, 4));//设置画笔形式
	painter.save();
	//for (int i = 0; i < m_shapeHandler_h->m_allParts.size(); i++)
	//{
	//	CShapePart *shapePart = m_shapeHandler_h->m_allParts[i];
	//	if (shapePart->m_lbv_isSelected)
	//	{
	//		for (int j = 0; j < shapePart->edges.size(); j++)
	//		{
	//			Edge e = shapePart->edges[j];
	//			if (e.m_lbv_isVisible)
	//			{
	//				painter.drawEllipse(QPoint(e.m_lc2v_endPt2D[0][0], e.m_lc2v_endPt2D[0][1]), 2, 2);//画圆
	//				painter.drawEllipse(QPoint(e.m_lc2v_endPt2D[1][0], e.m_lc2v_endPt2D[1][1]), 2, 2);//画圆
	//			}
	//		}

	//	}
	//}
	painter.restore();
}

void CDisplayWidget::DrawEditRelationsStroke(QPainter& painter)
{
	if (m_relationStrokeAL.size() == 0)
		return;
	painter.save();
	painter.setRenderHint(QPainter::Antialiasing, true);
	QColor c(200, 0, 0, 200);
	QPen thisPen(QBrush(c), 5, Qt::SolidLine, Qt::RoundCap);
	thisPen.setJoinStyle(Qt::RoundJoin);
	painter.setPen(thisPen);
	QPainterPath path;
	path.moveTo(m_relationStrokeAL[0][0], m_relationStrokeAL[0][1]);
	for (int i = 1; i < m_relationStrokeAL.size(); ++i)
	{
		path.lineTo(m_relationStrokeAL[i][0], m_relationStrokeAL[i][1]);
		path.moveTo(m_relationStrokeAL[i][0], m_relationStrokeAL[i][1]);
	}
	painter.drawPath(path);
	painter.restore();
}

void CDisplayWidget::UpdateRelations()
{
	//std::cout << "=============================gdv_unitThreshold is " << gdv_unitThreshold << "\n";
	//std::cout << "=================UpdateRelations start\n";
	QTime startTime = QTime::currentTime(); //记录时间
	if (m_mouseOperateType==SC::RecSelect)
	{
		m_partShapeHandler_h.ComputeEdgeAndNormal(this->camera());//计算边和法线
		//m_partShapeHandler_h
		ComputeRelationsByRansac(&m_partShapeHandler_h);
		//RemoveErrorRelaions(m_partShapeHandler_h);
	}
	else
	{
		m_shapeHandler_h->ComputeEdgeAndNormal(this->camera());//计算边和法线
		//m_shapeHandler_h
		ComputeRelationsByRansac(m_shapeHandler_h);
		//RemoveErrorRelaions(*m_shapeHandler_h);
	}
	//把m_recordRelations排个序
	SortRelaions();
	AddAndDeleteRelaions();
	//添加spacingRelation
	if (m_isShowSpacingRelations)
	{
		ComputeSpacingRelations();
	}
	emit UpdateRelationsSender();
	QTime stopTime = QTime::currentTime();
	int elapsed = startTime.msecsTo(stopTime);
	/*std::cout << "UpdateRelations time is " << elapsed << "\n";*/
}

void CDisplayWidget::ComputeRelationsByRansac(CShapeHandler* handler)
{
	if (handler->m_allParts.size() < 2)
		return;
	//检测关系
	m_recordRelations.clear();	//根据可视化的结果得到两两shape的关系
	std::vector<std::vector<Edge>> m_relationsSets;
	m_relationsSets = handler->GetEdgeRelationsWithRansac();

	if (m_relationsSets.empty())
		return;
	for (int i = 0; i < m_relationsSets.size(); i++)
	{
		//求直线
		std::vector<cv::Vec2d> points;
		for (int j = 0; j < m_relationsSets[i].size(); j++)
		{
			for (int k = 0; k < m_relationsSets[i][j].m_points.size(); k++)
			{
				points.push_back(m_relationsSets[i][j].m_points[k]);
			}
		}
		if (points.empty())
		{
			return;
		}
		Point2D32f *subPoints = new Point2D32f[points.size()];
		for (int m = 0; m < points.size(); m++)
		{
			subPoints[m].x = points[m][0];
			subPoints[m].y = points[m][1];
		}
		float tempLine[4];
		Utils::FitLine2D(subPoints, points.size(), tempLine);
		for (int j = 0; j < m_relationsSets[i].size(); j++)
		{
			m_relationsSets[i][j].m_line[0] = tempLine[0];
			m_relationsSets[i][j].m_line[1] = tempLine[1];
			m_relationsSets[i][j].m_line[2] = tempLine[2];
			m_relationsSets[i][j].m_line[3] = tempLine[3];
			m_relationsSets[i][j].ComputeEndPt2DProjectedVertex();
		}
		////可视化
		int len = m_relationsSets[i].size();
		Edge *lines = new Edge[len];
		for (int l = 0; l < len; l++)
		{
			lines[l] = m_relationsSets[i][l];
		}
		for (int m = 0; m < len; m++)
		{
			for (int n = m + 1; n < len; n++)
			{
				if (lines[m].m_point_dis[0]>lines[n].m_point_dis[0])
				{
					Edge e = lines[m];
					lines[m] = lines[n];
					lines[n] = e;
				}
			}
		}
		for (int k = 0; k < len - 1; k++)
		{
			ShapeRelation shapeRelation;
			shapeRelation.edges[0] = lines[k];
			shapeRelation.edges[1] = lines[k + 1];

			QPointF point1, point2;
			if (lines[k].m_point_dis[0] < lines[k].m_point_dis[1])
			{
				point1.setX(lines[k].m_projectPt2D[1][0]); point1.setY(lines[k].m_projectPt2D[1][1]);
			}
			else
			{
				point1.setX(lines[k].m_projectPt2D[0][0]); point1.setY(lines[k].m_projectPt2D[0][1]);
			}
			if (lines[k + 1].m_point_dis[0] < lines[k + 1].m_point_dis[1])
			{
				point2.setX(lines[k + 1].m_projectPt2D[0][0]); point2.setY(lines[k + 1].m_projectPt2D[0][1]);
			}
			else
			{
				point2.setX(lines[k + 1].m_projectPt2D[1][0]); point2.setY(lines[k + 1].m_projectPt2D[1][1]);
			}

			//painter.drawLine(point1, point2);
			RecordNo recordNo;
			QPointF point;
			for (int s = 0; s < handler->m_allParts.size(); s++)
			{
				for (int v = 0; v < handler->m_allParts[s]->edges.size(); v++)
				{
					point.setX(handler->m_allParts[s]->edges[v].m_projectPt2D[0][0]);
					point.setY(handler->m_allParts[s]->edges[v].m_projectPt2D[0][1]);
					if (point == point1)
					{
						recordNo.edge_no1 = v;
						recordNo.shape_no1 = s;
						recordNo.vertices_no1 = 0;
						break;
					}
					point.setX(handler->m_allParts[s]->edges[v].m_projectPt2D[1][0]);
					point.setY(handler->m_allParts[s]->edges[v].m_projectPt2D[1][1]);
					if (point == point1)
					{
						recordNo.edge_no1 = v;
						recordNo.shape_no1 = s;
						recordNo.vertices_no1 = 1;
						break;
					}
				}
				for (int v = 0; v < handler->m_allParts[s]->edges.size(); v++)
				{
					point.setX(handler->m_allParts[s]->edges[v].m_projectPt2D[0][0]);
					point.setY(handler->m_allParts[s]->edges[v].m_projectPt2D[0][1]);
					if (point == point2)
					{
						recordNo.edge_no2 = v;
						recordNo.shape_no2 = s;
						recordNo.vertices_no2 = 0;
						break;
					}
					point.setX(handler->m_allParts[s]->edges[v].m_projectPt2D[1][0]);
					point.setY(handler->m_allParts[s]->edges[v].m_projectPt2D[1][1]);
					if (point == point2)
					{
						recordNo.edge_no2 = v;
						recordNo.shape_no2 = s;
						recordNo.vertices_no2 = 1;
						break;
					}
				}
			}
			//m_recordNos.push_back(recordNo);
			shapeRelation.recordNo = recordNo;
			m_recordRelations.push_back(shapeRelation);
		}
	}
}

void CDisplayWidget::DrawRelitions(QPainter& painter)
{
	//用于delete关系
	m_recordShowRelaion.clear();
	m_recordShowRelaion = m_recordRelations;
	//m_recordRelations
	if (m_recordRelations.empty()) return;
	//关系的可视化
	if (m_recordRelations.size() > 0)
	{
		CShapeHandler* handler;
		if (m_mouseOperateType == SC::RecSelect)
		{
			handler=&m_partShapeHandler_h;
		}
		else
		{
			//m_shapeHandler_h
			handler=m_shapeHandler_h;

		}
		if (handler->m_allParts.empty())
			return;
		QPen thisPen;
		thisPen.setColor(QColor(0, 0, 255, 255));
		thisPen.setStyle(Qt::CustomDashLine);
		thisPen.setWidthF(3);//2

		QVector<qreal> dashes;
		qreal space = 4;
		dashes << 2 << space << 2 << space;

		thisPen.setDashPattern(dashes);
		//thisPen.
		painter.save();
		painter.setRenderHint(QPainter::Antialiasing, true);
		painter.setPen(thisPen);
		//
		QPointF point1, point2;
		for (int i = 0; i < m_recordRelations.size(); i++)
		{
			RecordNo no = m_recordRelations[i].recordNo;
			if (no.shape_no1<handler->m_allParts.size() && no.shape_no2<handler->m_allParts.size() && no.edge_no1<12 && no.edge_no2<12)//防止越界
			{
				point1.setX(handler->m_allParts[no.shape_no1]->edges[no.edge_no1].m_projectPt2D[no.vertices_no1][0]);
				point1.setY(handler->m_allParts[no.shape_no1]->edges[no.edge_no1].m_projectPt2D[no.vertices_no1][1]);
				point2.setX(handler->m_allParts[no.shape_no2]->edges[no.edge_no2].m_projectPt2D[no.vertices_no2][0]);
				point2.setY(handler->m_allParts[no.shape_no2]->edges[no.edge_no2].m_projectPt2D[no.vertices_no2][1]);
				painter.drawLine(point1, point2);
			}
		}
		painter.restore();
	}
}

void CDisplayWidget::RemoveErrorRelaions(CShapeHandler handler)
{
	//判断两个shape是否有两条以上的关系，用于旋转优化
	if (m_recordRelations.size() == 0)
		return;
	if (handler.m_allParts.size()<2)
		return;
	std::vector<ShapeRelation> relations=m_recordRelations;
	m_recordRelations.clear();//清除之前记录的关系
	//判断两个shape是否有两条以上的关系，用于旋转优化
	for (int i = 0; i < relations.size(); i++)
	{
		relations[i].is2relations = false;//初始化
	}
	for (int i = 0; i < relations.size(); i++)
	{
		ShapeRelation shapeRelation1 = relations[i];
		for (int j = 0; j < i; j++)
		{
			ShapeRelation shapeRelation2 = relations[j];
			if (shapeRelation2.edges[0].m_shapeIndex == shapeRelation1.edges[0].m_shapeIndex)
			{
				if (shapeRelation2.edges[1].m_shapeIndex == shapeRelation1.edges[1].m_shapeIndex)
				{
					double a = abs(shapeRelation1.edges[0].m_rotateNo - shapeRelation2.edges[0].m_rotateNo);
					double b = abs(shapeRelation1.edges[1].m_rotateNo - shapeRelation2.edges[1].m_rotateNo);
					if (a - b == 0)
					{
						relations[i].is2relations = true;
						break;
					}
					continue;
				}
			}
			if (shapeRelation2.edges[0].m_shapeIndex == shapeRelation1.edges[1].m_shapeIndex)
			{
				if (shapeRelation2.edges[1].m_shapeIndex == shapeRelation1.edges[0].m_shapeIndex)
				{
					double a = abs(shapeRelation1.edges[0].m_rotateNo - shapeRelation2.edges[1].m_rotateNo);
					double b = abs(shapeRelation1.edges[1].m_rotateNo - shapeRelation2.edges[0].m_rotateNo);
					if (a - b == 0)
					{
						relations[i].is2relations = true;
						break;
					}
					continue;
				}
			}
		}
	}
	std::vector<bool> floatings;//记录所有shape的悬浮状态
	for (int i = 0; i < handler.m_allParts.size(); i++)
	{
		floatings.push_back(handler.m_allParts[i]->m_isFloating);
	}
	int k = 0;
	for ( k = 0; k < relations.size();k++)
	{
		if (!floatings[relations[k].edges[0].m_shapeIndex]) break;
		if (!floatings[relations[k].edges[1].m_shapeIndex]) break;
	}
	if (k == relations.size())
	{
		floatings[0] = false;
	}
	
	handler.FixAllModels(camera());
	CLayoutRefiner layoutRefiner;
	layoutRefiner.SetData(&handler);
	//旋转优化
	layoutRefiner.RotationOptimizing(&relations, camera());
	//旋转优化结束
	std::vector<ShapeRelation> availablerelations;//可利用的关系
	for (int i = 0; i < relations.size(); i++)
	{
		availablerelations.push_back(relations[i]);
		if (!layoutRefiner.CheckErrorRelation(availablerelations, this->camera(), floatings))
			availablerelations.pop_back();
	}
	m_optRelations.clear();
	for (int i = 0; i < availablerelations.size(); i++)
	{
		m_optRelations.push_back(availablerelations[i]);
	}
	m_recordRelations.clear();
	for (int i = 0; i < m_optRelations.size(); i++)
	{
		m_recordRelations.push_back(m_optRelations[i]);
	}
}

void CDisplayWidget::ExecuteAlignmentOptimizing()
{
	/*if (m_mouseOperateType == SC::DeleteRelations || m_mouseOperateType == SC::AddRelations)
	{
		m_optRelations.clear();
		for (int i = 0; i < m_editRelations.size(); i++)
		{
			m_optRelations.push_back(m_editRelations[i]);
		}
	}*/
	CShapeHandler *handle;
	if (m_mouseOperateType == SC::RecSelect)
	{
		handle = &m_partShapeHandler_h;
	}
	else
	{
		handle = m_shapeHandler_h;
	}
	if (m_optRelations.size() == 0)
		return;
	if (handle == NULL)
		return;
	if (handle->m_allParts.size()<2)
		return;
	//CShapeHandler clone_handler = *m_shapeHandler_h;
	//选出正确的关系
	for (int i = 0; i < m_optRelations.size(); i++)
	{
		m_optRelations[i].is2relations = false;
	}
	for (int i = 0; i < m_optRelations.size(); i++)
	{
		ShapeRelation shapeRelation1 = m_optRelations[i];
		for (int j = 0; j < i; j++)
		{
			ShapeRelation shapeRelation2 = m_optRelations[j];
			if (shapeRelation2.edges[0].m_shapeIndex == shapeRelation1.edges[0].m_shapeIndex)
			{
				if (shapeRelation2.edges[1].m_shapeIndex == shapeRelation1.edges[1].m_shapeIndex)
				{
					double a = abs(shapeRelation1.edges[0].m_rotateNo - shapeRelation2.edges[0].m_rotateNo);
					double b = abs(shapeRelation1.edges[1].m_rotateNo - shapeRelation2.edges[1].m_rotateNo);
					if (a - b == 0)
					{
						m_optRelations[i].is2relations = true;
						//relations[i - 1].is2relations = true;
					}
					continue;
				}
			}
			if (shapeRelation2.edges[0].m_shapeIndex == shapeRelation1.edges[1].m_shapeIndex)
			{
				if (shapeRelation2.edges[1].m_shapeIndex == shapeRelation1.edges[0].m_shapeIndex)
				{
					double a = abs(shapeRelation1.edges[0].m_rotateNo - shapeRelation2.edges[1].m_rotateNo);
					double b = abs(shapeRelation1.edges[1].m_rotateNo - shapeRelation2.edges[0].m_rotateNo);
					if (a - b == 0)
					{
						m_optRelations[i].is2relations = true;
					}
					//relations[i].is2relations = true;
					continue;
				}
			}
		}

	}
	vector<bool> floatings;//记录所有shape的悬浮状态
	for (int i = 0; i < handle->m_allParts.size(); i++)
	{
		floatings.push_back(handle->m_allParts[i]->m_isFloating);
	}
	int k = 0;
	for (k = 0; k < m_optRelations.size(); k++)
	{
		if (!floatings[m_optRelations[k].edges[0].m_shapeIndex]) break;
		if (!floatings[m_optRelations[k].edges[1].m_shapeIndex]) break;
	}
	if (k == m_optRelations.size())
	{
		floatings[0] = false;
		m_optRelations.clear();
		return;
	}
	emit BackupDataSender();
	handle->FixAllModels(camera());
	CLayoutRefiner layoutRefiner;
	layoutRefiner.SetData(handle);
	//
	layoutRefiner.ExecuteAlignmentOptimizing(m_optRelations, this->camera(), &floatings);
	FixModelsByFloatIndex(floatings,handle);
	if (m_isShowSpacingRelations&&!m_SpacingRelations.empty())
	{
		m_shapeHandler_h->ComputeEdgeAndNormal(camera());
		m_optRelations = UpdateVecRelations(m_optRelations);
		//GetSpacingRelations(m_optRelations);
		//CLayoutRefiner layoutRefiner1;
		//layoutRefiner1.SetData(handle);
		layoutRefiner.ExecuteSpacingOptimizing(m_optRelations, this->camera(), m_SpacingRelations);
	}
	m_optRelations.clear();
	emit BackupDataSender();
	//m_shapeHandler_h->DestoryAllParts();
	/*for (int i = 0; i < clone_handler.m_allParts.size();i++)
	{
	CShapePart shapePart = *clone_handler.m_allParts[i];
	m_shapeHandler_h->AddNewPart(shapePart);
	}*/
	repaint();

	
}

void CDisplayWidget::FixModelsByFloatIndex(vector<bool> floatIndex, CShapeHandler* handler)
{
	for (int i = 0; i < handler->m_allParts.size(); i++)
	{
		if (floatIndex[i])
		{
			handler->m_allParts[i]->m_isFloating = true;
			GLdouble camera_matrix[16];
			cv::Mat mat_m = cv::Mat::zeros(4, 4, CV_64F);
			for (int m = 0; m < 4; m++)
			{
				for (int n = 0; n < 4; n++)
				{
					mat_m.at<double>(m, n) = camera()->frame()->matrix()[m * 4 + n];
				}
			}
			mat_m = mat_m.inv();
			for (int m = 0; m < 4; m++)
			{
				for (int n = 0; n < 4; n++)
				{
					camera_matrix[m * 4 + n] = mat_m.at<double>(m, n);
				}
			}
			GLdouble matrix[16];
			for (int k = 0; k < 16; k++)
			{
				matrix[k] = handler->m_allParts[i]->m_frame->matrix()[k];
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
			handler->m_allParts[i]->m_frame->setFromMatrix(matrix);
			//设置参考frame为相机，使位置处于悬浮状态
			handler->m_allParts[i]->m_frame->setReferenceFrame(camera()->frame());
		}
	}
}

void CDisplayWidget::DrawShapeNo(QPainter& painter)
{
	for (int i = 0; i < m_shapeHandler_h->m_allParts.size();i++)
	{
		m_shapeHandler_h->m_allParts[i]->DrawNo(painter);
	}
}

void CDisplayWidget::DrawGrid(float size /*= 1.0f*/, int nbSubdivisions /*= 10*/)
{
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	glEnable(GL_MULTISAMPLE);
	glPushMatrix();
	qglviewer::Quaternion thisQuaternion(qglviewer::Vec(1, 0, 0), M_PI * 0.5);
	glMultMatrixd(thisQuaternion.matrix());
	glColor3d(0.1, 0.1, 0.1);
	//glColor4ub(180, 180, 180, 255);
	//glColor4ub(50, 50, 50, 255)
	QGLViewer::drawGrid(size, nbSubdivisions);
	glPopMatrix();
}

CShapeHandler CDisplayWidget::GetPreviewResult()
{
	CShapeHandler result;
	result.DestoryAllParts();
	CShapeHandler *handle;
	if (m_mouseOperateType == SC::RecSelect)
	{
		handle = &m_partShapeHandler_h;
	}
	else
	{
		handle = m_shapeHandler_h;
	}
	std::vector<ShapeRelation> optRelations = m_optRelations;
	std::vector<std::vector<RecordNo>> spacingRelations = m_SpacingRelations;
	if (optRelations.size() == 0 || handle->m_allParts.size()<2)
	{
		for (int i = 0; i < handle->m_allParts.size(); i++)
		{
			CShapePart* newPart = new CShapePart(*(handle->m_allParts[i]));
			if (!newPart->m_isFloating)
			result.m_allParts.push_back(newPart);
		}
		return result;
	}
		
	if (handle == NULL)
		return result;
	/*if (handle->m_allParts.size()<2)
		return result;*/
	for (int i = 0; i < handle->m_allParts.size(); i++)
	{
		CShapePart* newPart = new CShapePart(*(handle->m_allParts[i]));
		result.m_allParts.push_back(newPart);
	}
	//选出正确的关系
	for (int i = 0; i < optRelations.size(); i++)
	{
		optRelations[i].is2relations = false;
	}
	for (int i = 0; i < optRelations.size(); i++)
	{
		ShapeRelation shapeRelation1 = optRelations[i];
		for (int j = 0; j < i; j++)
		{
			ShapeRelation shapeRelation2 = optRelations[j];
			if (shapeRelation2.edges[0].m_shapeIndex == shapeRelation1.edges[0].m_shapeIndex)
			{
				if (shapeRelation2.edges[1].m_shapeIndex == shapeRelation1.edges[1].m_shapeIndex)
				{
					double a = abs(shapeRelation1.edges[0].m_rotateNo - shapeRelation2.edges[0].m_rotateNo);
					double b = abs(shapeRelation1.edges[1].m_rotateNo - shapeRelation2.edges[1].m_rotateNo);
					if (a - b == 0)
					{
						optRelations[i].is2relations = true;
						//relations[i - 1].is2relations = true;
					}
					continue;
				}
			}
			if (shapeRelation2.edges[0].m_shapeIndex == shapeRelation1.edges[1].m_shapeIndex)
			{
				if (shapeRelation2.edges[1].m_shapeIndex == shapeRelation1.edges[0].m_shapeIndex)
				{
					double a = abs(shapeRelation1.edges[0].m_rotateNo - shapeRelation2.edges[1].m_rotateNo);
					double b = abs(shapeRelation1.edges[1].m_rotateNo - shapeRelation2.edges[0].m_rotateNo);
					if (a - b == 0)
					{
						optRelations[i].is2relations = true;
					}
					//relations[i].is2relations = true;
					continue;
				}
			}
		}

	}
	vector<bool> floatings;//记录所有shape的悬浮状态
	for (int i = 0; i < result.m_allParts.size(); i++)
	{
		floatings.push_back(result.m_allParts[i]->m_isFloating);
	}
	int k = 0;
	for (k = 0; k < optRelations.size(); k++)
	{
		if (!floatings[optRelations[k].edges[0].m_shapeIndex]) break;
		if (!floatings[optRelations[k].edges[1].m_shapeIndex]) break;
	}
	if (k == optRelations.size())
	{
		floatings[0] = false;
		//result.DestoryAllParts();
		return result;
	}
	result.FixAllModels(camera());
	CLayoutRefiner layoutRefiner;
	layoutRefiner.SetData(&result);
	layoutRefiner.ExecuteAlignmentOptimizing(optRelations, this->camera(), &floatings);
	FixModelsByFloatIndex(floatings, &result);
	if (m_isShowSpacingRelations&&!m_SpacingRelations.empty())
	{
		result.ComputeEdgeAndNormal(camera());
		optRelations = UpdateVecRelations(optRelations, &result);
		layoutRefiner.ExecuteSpacingOptimizing(optRelations, this->camera(), spacingRelations);
	}
	return result;
}

void CDisplayWidget::EditRelations()
{
	m_stopRelationUpdate = true;
	if (m_optRelations.empty())
		return;
	m_editRelations.clear();
	for (int i = 0; i < m_optRelations.size(); i++)
	{
		m_editRelations.push_back(m_optRelations[i]);
	}
	m_optRelations.clear();
	m_recordRelations.clear();
}

void CDisplayWidget::DrawEditRelitions(QPainter& painter)
{
	//m_editRelations
	if (m_editRelations.empty()) return;
	//编辑关系的可视化
	if (m_editRelations.size() > 0)
	{
		CShapeHandler* handler;
		if (m_mouseOperateType == SC::RecSelect)
		{
			handler = &m_partShapeHandler_h;
		}
		else
		{
			//m_shapeHandler_h
			handler = m_shapeHandler_h;

		}
		if (handler->m_allParts.empty())
			return;
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
		//
		QPointF point1, point2;
		for (int i = 0; i < m_editRelations.size(); i++)
		{
			RecordNo no = m_editRelations[i].recordNo;
			if (no.shape_no1 < handler->m_allParts.size() && no.shape_no2 < handler->m_allParts.size() && no.edge_no1 < 12 && no.edge_no2 < 12)//防止越界
			{
				point1.setX(handler->m_allParts[no.shape_no1]->edges[no.edge_no1].m_projectPt2D[no.vertices_no1][0]);
				point1.setY(handler->m_allParts[no.shape_no1]->edges[no.edge_no1].m_projectPt2D[no.vertices_no1][1]);
				point2.setX(handler->m_allParts[no.shape_no2]->edges[no.edge_no2].m_projectPt2D[no.vertices_no2][0]);
				point2.setY(handler->m_allParts[no.shape_no2]->edges[no.edge_no2].m_projectPt2D[no.vertices_no2][1]);
				painter.drawLine(point1, point2);
			}
		}
		painter.restore();
	}
}

void CDisplayWidget::AddRelation()
{
	if (m_relationStrokeAL.size() < 2)
		return;
	/*if (m_editRelations.empty())
		return;*/
	CShapeHandler* handler;
	if (m_mouseOperateType == SC::RecSelect)
	{
		handler = &m_partShapeHandler_h;
	}
	else
	{
		//m_shapeHandler_h
		handler = m_shapeHandler_h;

	}
	if (handler->m_allParts.empty())
		return;
	
	//添加关系
	bool isAddRelaion = true;
	//编号
	handler->ComputeEdgeAndNormal(this->camera());//计算边和法线
	for (int i = 0; i < handler->m_allParts.size(); i++)
	{
		for (int j = 0; j < handler->m_allParts[i]->edges.size(); j++)
		{
			handler->m_allParts[i]->edges[j].m_shapeIndex = i;
		}
	}
	cv::Vec2d startPoint = m_relationStrokeAL.front();
	cv::Vec2d endPoint = m_relationStrokeAL.back();
	//起点
	Edge startEdge;
	double dis = 100;
	for (int i = 0; i < handler->m_allParts.size(); i++)
	{
		for (int j = 0; j < handler->m_allParts[i]->edges.size(); j++)
		{
			Edge e = handler->m_allParts[i]->edges[j];
			if (e.m_isVisible)
			{
				for (int k = 0; k < e.m_points.size();k++)
				{
					if (dis > Utils::Calculate2DTwoPointDistance(startPoint, e.m_points[k]))
					{
						startEdge = e;
						dis = Utils::Calculate2DTwoPointDistance(startPoint, e.m_points[k]);
					}
				}
				
			}
		}
	}
	if (dis > 30)
		isAddRelaion = false;
	//终点
	Edge endEdge;
	dis = 100;
	for (int i = 0; i < handler->m_allParts.size(); i++)
	{
		for (int j = 0; j < handler->m_allParts[i]->edges.size(); j++)
		{
			Edge e = handler->m_allParts[i]->edges[j];
			if (e.m_isVisible)
			{
				for (int k = 0; k < e.m_points.size();k++)
				{
					if (dis > Utils::Calculate2DTwoPointDistance(endPoint, e.m_points[k]))
					{
						endEdge = e;
						dis = Utils::Calculate2DTwoPointDistance(endPoint, e.m_points[k]);
					}
				}
				
			}
		}
	}
	
	if (dis > 30)
		isAddRelaion=false;
	if (startEdge.m_shapeIndex == endEdge.m_shapeIndex)
		isAddRelaion = false;
	if (abs(startEdge.m_dir3D.dot(endEdge.m_dir3D)) < 0.9)
		isAddRelaion = false;
	//距离限制
	float distance = handler->ComputEdgeToLineDis(startEdge, endEdge);//计算两条边的距离
	if (distance > 40)
		isAddRelaion = false;
	/*for (int i = 0; i < m_editRelations.size();i++)
	{
		ShapeRelation r = m_editRelations[i];
		Edge e1 = r.edges[0];
		Edge e2 = r.edges[1];
		if (e1.m_shapeIndex == startEdge.m_shapeIndex&&e1.m_shapeEdgeNo == startEdge.m_shapeEdgeNo)
			return;
		if (e2.m_shapeIndex == startEdge.m_shapeIndex&&e2.m_shapeEdgeNo == startEdge.m_shapeEdgeNo)
			return;
		if (e1.m_shapeIndex == endEdge.m_shapeIndex&&e1.m_shapeEdgeNo == endEdge.m_shapeEdgeNo)
			return;
		if (e2.m_shapeIndex == endEdge.m_shapeIndex&&e2.m_shapeEdgeNo == endEdge.m_shapeEdgeNo)
			return;
	}*/
	if (isAddRelaion)
	{
		ShapeRelation shapeRelation;
		shapeRelation.edges[0] = startEdge;
		shapeRelation.edges[1] = endEdge;
		RecordNo recordNo;
		double dis1 = Utils::Calculate2DTwoPointDistance(startEdge.m_projectPt2D[0], endEdge.m_projectPt2D[0]);
		double dis2 = Utils::Calculate2DTwoPointDistance(startEdge.m_projectPt2D[0], endEdge.m_projectPt2D[1]);
		double dis3 = Utils::Calculate2DTwoPointDistance(startEdge.m_projectPt2D[1], endEdge.m_projectPt2D[0]);
		double dis4 = Utils::Calculate2DTwoPointDistance(startEdge.m_projectPt2D[1], endEdge.m_projectPt2D[1]);
		double min = dis1;
		cv::Vec2d point1, point2;
		if (dis2 < min)
			min = dis2;
		if (dis3 < min)
			min = dis3;
		if (dis4 < min)
			min = dis4;
		if (dis1 == min)
		{
			point1 = startEdge.m_projectPt2D[0];
			point2 = endEdge.m_projectPt2D[0];
		}
		if (dis2 == min)
		{
			point1 = startEdge.m_projectPt2D[0];
			point2 = endEdge.m_projectPt2D[1];
		}
		if (dis3 == min)
		{
			point1 = startEdge.m_projectPt2D[1];
			point2 = endEdge.m_projectPt2D[0];
		}
		if (dis4 == min)
		{
			point1 = startEdge.m_projectPt2D[1];
			point2 = endEdge.m_projectPt2D[1];
		}
		QPointF point, p1, p2;
		p1.setX(point1[0]);
		p1.setY(point1[1]);
		p2.setX(point2[0]);
		p2.setY(point2[1]);
		for (int s = 0; s < handler->m_allParts.size(); s++)
		{
			for (int v = 0; v < handler->m_allParts[s]->edges.size(); v++)
			{
				point.setX(handler->m_allParts[s]->edges[v].m_projectPt2D[0][0]);
				point.setY(handler->m_allParts[s]->edges[v].m_projectPt2D[0][1]);
				if (point == p1)
				{
					recordNo.edge_no1 = v;
					recordNo.shape_no1 = s;
					recordNo.vertices_no1 = 0;
					break;
				}
				point.setX(handler->m_allParts[s]->edges[v].m_projectPt2D[1][0]);
				point.setY(handler->m_allParts[s]->edges[v].m_projectPt2D[1][1]);
				if (point == p1)
				{
					recordNo.edge_no1 = v;
					recordNo.shape_no1 = s;
					recordNo.vertices_no1 = 1;
					break;
				}
			}
			for (int v = 0; v < handler->m_allParts[s]->edges.size(); v++)
			{
				point.setX(handler->m_allParts[s]->edges[v].m_projectPt2D[0][0]);
				point.setY(handler->m_allParts[s]->edges[v].m_projectPt2D[0][1]);
				if (point == p2)
				{
					recordNo.edge_no2 = v;
					recordNo.shape_no2 = s;
					recordNo.vertices_no2 = 0;
					break;
				}
				point.setX(handler->m_allParts[s]->edges[v].m_projectPt2D[1][0]);
				point.setY(handler->m_allParts[s]->edges[v].m_projectPt2D[1][1]);
				if (point == p2)
				{
					recordNo.edge_no2 = v;
					recordNo.shape_no2 = s;
					recordNo.vertices_no2 = 1;
					break;
				}
			}
		}
		shapeRelation.recordNo = recordNo;
		//把该关系放进editRelaion
		m_recordRelations.clear();
		m_recordRelations.push_back(shapeRelation);
		for (int i = 0; i < m_editRelations.size(); i++)
		{
			m_recordRelations.push_back(m_editRelations[i]);
		}

		//RemoveErrorRelaions(*handler);
		/*if (!RelaionIsTrue(*handler))
		{
		m_recordRelations.clear();
		m_optRelations.clear();
		return;
		}*/
		RemoveErrorRelaions(*handler);
		m_recordRelations.clear();
		m_editRelations.clear();
		for (int i = 0; i < m_optRelations.size(); i++)
		{
			m_editRelations.push_back(m_optRelations[i]);
		}
		emit UpdateRelationsSender();
	}
	else
	{
		//删除关系
		cv::Vec3d pointA, pointB, pointC, pointD;
		pointC[0] = m_relationStrokeAL.front()[0];
		pointC[1] = m_relationStrokeAL.front()[1];
		pointC[2] = 0;
		pointD[0] = m_relationStrokeAL.back()[0];
		pointD[1] = m_relationStrokeAL.back()[1];
		pointD[2] = 0;

		for (std::vector<ShapeRelation>::iterator it = m_editRelations.begin(); it != m_editRelations.end();)
		{
			RecordNo no = (*it).recordNo;
			if (no.shape_no1 < handler->m_allParts.size() && no.shape_no2 < handler->m_allParts.size() && no.edge_no1 < 12 && no.edge_no2 < 12)//防止越界
			{
				pointA[0] = handler->m_allParts[no.shape_no1]->edges[no.edge_no1].m_projectPt2D[no.vertices_no1][0];
				pointA[1] = handler->m_allParts[no.shape_no1]->edges[no.edge_no1].m_projectPt2D[no.vertices_no1][1];
				pointA[2] = 0;
				pointB[0] = (handler->m_allParts[no.shape_no2]->edges[no.edge_no2].m_projectPt2D[no.vertices_no2][0]);
				pointB[1] = (handler->m_allParts[no.shape_no2]->edges[no.edge_no2].m_projectPt2D[no.vertices_no2][1]);
				pointB[2] = 0;
				cv::Vec3d pointAB = pointB - pointA;
				cv::Vec3d pointCD = pointD - pointC;
				cv::Vec3d pointAC = pointC - pointA;
				cv::Vec3d pointAD = pointD - pointA;
				cv::Vec3d pointCA = pointA - pointC;
				cv::Vec3d pointCB = pointB - pointC;
				//
				cv::Vec3d pointABAC = pointAB.cross(pointAC);
				cv::Vec3d pointABAD = pointAB.cross(pointAD);
				//
				cv::Vec3d pointCDCA = pointCD.cross(pointCA);
				cv::Vec3d pointCDCB = pointCD.cross(pointCB);
				if (pointABAC.dot(pointABAD) == 0 || pointABAC.dot(pointABAD) < 0)
				{
					if (pointCDCA.dot(pointCDCB) == 0 || pointCDCA.dot(pointCDCB) < 0)
					{
						it = m_editRelations.erase(it);
					}
					else
					{
						it++;
					}
				}
				else
				{
					it++;
				}
			}
		}
		//检测编辑后的关系，并且把优化结果发送给preview

		m_recordRelations.clear();
		for (int i = 0; i < m_editRelations.size(); i++)
		{
			m_recordRelations.push_back(m_editRelations[i]);
		}
		RemoveErrorRelaions(*handler);
		emit UpdateRelationsSender();
	}
}

void CDisplayWidget::DeleteRelation()
{
	if (m_relationStrokeAL.size()<2)
		return ;
	if (m_editRelations.empty())
		return;
	CShapeHandler* handler;
	if (m_mouseOperateType == SC::RecSelect)
	{
		handler = &m_partShapeHandler_h;
	}
	else
	{
		//m_shapeHandler_h
		handler = m_shapeHandler_h;

	}
	if (handler->m_allParts.empty())
		return;
	cv::Vec3d pointA, pointB, pointC, pointD;
	pointC[0] = m_relationStrokeAL.front()[0];
	pointC[1] = m_relationStrokeAL.front()[1];
	pointC[2] = 0;
	pointD[0] = m_relationStrokeAL.back()[0];
	pointD[1] = m_relationStrokeAL.back()[1];
	pointD[2] = 0;
	
	for (std::vector<ShapeRelation>::iterator it = m_editRelations.begin(); it != m_editRelations.end();)
	{
		RecordNo no = (*it).recordNo;
		if (no.shape_no1 < handler->m_allParts.size() && no.shape_no2 < handler->m_allParts.size() && no.edge_no1 < 12 && no.edge_no2 < 12)//防止越界
		{
			pointA[0] = handler->m_allParts[no.shape_no1]->edges[no.edge_no1].m_projectPt2D[no.vertices_no1][0];
			pointA[1] = handler->m_allParts[no.shape_no1]->edges[no.edge_no1].m_projectPt2D[no.vertices_no1][1];
			pointA[2] = 0;
			pointB[0] = (handler->m_allParts[no.shape_no2]->edges[no.edge_no2].m_projectPt2D[no.vertices_no2][0]);
			pointB[1] = (handler->m_allParts[no.shape_no2]->edges[no.edge_no2].m_projectPt2D[no.vertices_no2][1]);
			pointB[2] = 0;
			cv::Vec3d pointAB = pointB - pointA;
			cv::Vec3d pointCD = pointD - pointC;
			cv::Vec3d pointAC = pointC - pointA;
			cv::Vec3d pointAD = pointD - pointA;
			cv::Vec3d pointCA = pointA - pointC;
			cv::Vec3d pointCB = pointB - pointC;
			//
			cv::Vec3d pointABAC = pointAB.cross(pointAC);
			cv::Vec3d pointABAD = pointAB.cross(pointAD);
			//
			cv::Vec3d pointCDCA = pointCD.cross(pointCA);
			cv::Vec3d pointCDCB = pointCD.cross(pointCB);
			if (pointABAC.dot(pointABAD) == 0 || pointABAC.dot(pointABAD) < 0)
			{
				if (pointCDCA.dot(pointCDCB) == 0 || pointCDCA.dot(pointCDCB) < 0)
				{
					it = m_editRelations.erase(it);
				}
				else
				{
					it++;
				}
			}
			else
			{
				it++;
			}
		}
	}
	//检测编辑后的关系，并且把优化结果发送给preview
	m_recordRelations.clear();
	for (int i = 0; i < m_editRelations.size(); i++)
	{
		m_recordRelations.push_back(m_editRelations[i]);
	}
	RemoveErrorRelaions(*handler);
	emit UpdateRelationsSender();
}

bool CDisplayWidget::RelaionIsTrue(CShapeHandler handler)
{
	//判断两个shape是否有两条以上的关系，用于旋转优化
	if (m_recordRelations.size() == 0)
		return false;
	if (handler.m_allParts.size()<2)
		return false;

	std::vector<ShapeRelation> relations = m_recordRelations;
	m_recordRelations.clear();//清除之前记录的关系
	//通过两条边的三维方向计算距离，用于关系好坏的排序
	for (int i = 0; i < relations.size(); i++)
	{
		Edge e1, e2;
		e1 = relations[i].edges[0];
		e2 = relations[i].edges[1];
		relations[i].dis = abs(e1.m_dir3D.dot(e2.m_dir3D));
	}
	//根据距离排序，距离越大关系越好
	ShapeRelation temp;
	for (int i = 0; i < relations.size() - 1; i++)
	{
		temp = relations[i];
		for (int j = relations.size() - 1; j > i; j--)
		{
			if (temp.dis < relations[j].dis)
			{
				relations[i] = relations[j];
				relations[j] = temp;
				temp = relations[i];
			}
		}
	}
	//判断两个shape是否有两条以上的关系，用于旋转优化
	for (int i = 0; i < relations.size(); i++)
	{
		relations[i].is2relations = false;//初始化
	}
	for (int i = 0; i < relations.size(); i++)
	{
		ShapeRelation shapeRelation1 = relations[i];
		for (int j = 0; j < i; j++)
		{
			ShapeRelation shapeRelation2 = relations[j];
			if (shapeRelation2.edges[0].m_shapeIndex == shapeRelation1.edges[0].m_shapeIndex)
			{
				if (shapeRelation2.edges[1].m_shapeIndex == shapeRelation1.edges[1].m_shapeIndex)
				{
					double a = abs(shapeRelation1.edges[0].m_rotateNo - shapeRelation2.edges[0].m_rotateNo);
					double b = abs(shapeRelation1.edges[1].m_rotateNo - shapeRelation2.edges[1].m_rotateNo);
					if (a - b == 0)
					{
						relations[i].is2relations = true;
						break;
					}
					continue;
				}
			}
			if (shapeRelation2.edges[0].m_shapeIndex == shapeRelation1.edges[1].m_shapeIndex)
			{
				if (shapeRelation2.edges[1].m_shapeIndex == shapeRelation1.edges[0].m_shapeIndex)
				{
					double a = abs(shapeRelation1.edges[0].m_rotateNo - shapeRelation2.edges[1].m_rotateNo);
					double b = abs(shapeRelation1.edges[1].m_rotateNo - shapeRelation2.edges[0].m_rotateNo);
					if (a - b == 0)
					{
						relations[i].is2relations = true;
						break;
					}
					continue;
				}
			}
		}
	}
	std::vector<bool> floatings;//记录所有shape的悬浮状态
	for (int i = 0; i < handler.m_allParts.size(); i++)
	{
		floatings.push_back(handler.m_allParts[i]->m_isFloating);
	}
	int k = 0;
	for (k = 0; k < relations.size(); k++)
	{
		if (!floatings[relations[k].edges[0].m_shapeIndex]) break;
		if (!floatings[relations[k].edges[1].m_shapeIndex]) break;
	}
	if (k == relations.size())
	{
		floatings[0] = false;
	}
	handler.FixAllModels(camera());
	CLayoutRefiner layoutRefiner;
	layoutRefiner.SetData(&handler);
	//旋转优化
	layoutRefiner.RotationOptimizing(&relations, camera());
	//旋转优化结束
	if (!layoutRefiner.CheckErrorRelation(relations, this->camera(), floatings))
		return false;
	m_optRelations.clear();
	for (int i = 0; i < relations.size(); i++)
	{
		m_optRelations.push_back(relations[i]);
	}
	m_recordRelations.clear();
	for (int i = 0; i < m_optRelations.size(); i++)
	{
		m_recordRelations.push_back(m_optRelations[i]);
	}
	return true;
}

bool CDisplayWidget::TwoRelaionIsEqual(ShapeRelation relation1, ShapeRelation relation2)
{
	
	if (relation1.edges[0].m_shapeId == relation2.edges[0].m_shapeId && relation1.edges[1].m_shapeId == relation2.edges[1].m_shapeId)
	{
		if (relation1.edges[0].m_edgeId == relation2.edges[0].m_edgeId && relation1.edges[1].m_edgeId == relation2.edges[1].m_edgeId)
		{
			return true;
		}
	}
	if (relation1.edges[0].m_shapeId == relation2.edges[1].m_shapeId  && relation1.edges[1].m_shapeId == relation2.edges[0].m_shapeId)
	{
		if (relation1.edges[0].m_edgeId == relation2.edges[1].m_edgeId  && relation1.edges[1].m_edgeId == relation2.edges[0].m_edgeId)
		{
			return true;
		}
	}
	/*if (Utils::CompareTwoRecordNo(relation1.recordNo,relation2.recordNo))
	{
		return true;
	}
	if (relation1.edges[0].m_shapeIndex == relation2.edges[0].m_shapeIndex && relation1.edges[1].m_shapeIndex == relation2.edges[1].m_shapeIndex)
	{
		if (relation1.edges[0].m_shapeEdgeNo == relation2.edges[0].m_shapeEdgeNo && relation1.edges[1].m_shapeEdgeNo == relation2.edges[1].m_shapeEdgeNo)
		{
			return true;
		}
	}
	if (relation1.edges[0].m_shapeIndex == relation2.edges[1].m_shapeIndex  && relation1.edges[1].m_shapeIndex == relation2.edges[0].m_shapeIndex)
	{
		if (relation1.edges[0].m_shapeEdgeNo == relation2.edges[1].m_shapeEdgeNo  && relation1.edges[1].m_shapeEdgeNo == relation2.edges[0].m_shapeEdgeNo)
		{
			return true;
		}
	}*/
	return false;
}

void CDisplayWidget::AddAndDeleteRelaions()
{
	//把record赋值给opt
	m_optRelations = m_recordRelations;
	CShapeHandler* handler;
	handler = m_shapeHandler_h;
	m_editRelationType = 0;//没有编辑关系
	if (handler->m_allParts.empty())
		return;
	if (m_relationStrokeAL.size() < 2)
	{
		//std::cout << "=================m_relationStrokeAL start\n";
		GetFinalOptRelaions();
		m_recordRelations.clear();
		for (int i = 0; i < m_optRelations.size(); i++)
		{
			m_recordRelations.push_back(m_optRelations[i]);
		}
		//QTime startTime = QTime::currentTime(); //记录时间
		RemoveErrorRelaions(*handler);
		emit UpdateRelationsSender();
		//std::cout << "=================m_relationStrokeAL end\n";
		return;
	}
	
	GetFinalOptRelaions();
	//添加关系
	bool isAddRelaion = true;
	//编号
	handler->ComputeEdgeAndNormal(this->camera());//计算边和法线
	for (int i = 0; i < handler->m_allParts.size(); i++)
	{
		for (int j = 0; j < handler->m_allParts[i]->edges.size(); j++)
		{
			handler->m_allParts[i]->edges[j].m_shapeIndex = i;
		}
	}
	cv::Vec2d startPoint = m_relationStrokeAL.front();
	cv::Vec2d endPoint = m_relationStrokeAL.back();


	//////////////////////////////////////////////////////////////////////////
	//添加关系修改，根据方向筛选候选边
	vector<Edge> candidataStartEdges;
	for (int i = 0; i < handler->m_allParts.size(); i++)
	{
		for (int j = 0; j < handler->m_allParts[i]->edges.size(); j++)
		{
			Edge e = handler->m_allParts[i]->edges[j];
			if (e.m_isVisible)
			{
				Edge e1=e;
				e1.m_point_edge_dis = 100;
				for (int k = 0; k < e.m_points.size(); k++)
				{
					if (e1.m_point_edge_dis > Utils::Calculate2DTwoPointDistance(startPoint, e.m_points[k]))
					{
						e1.m_point_edge_dis = Utils::Calculate2DTwoPointDistance(startPoint, e.m_points[k]);//m_point_edge_dis初始值-1
					}
				}
				if (e1.m_point_edge_dis<30&&e1.m_point_edge_dis!=-1)
				{
					candidataStartEdges.push_back(e1);
				}

			}
		}
	}
	vector<Edge> candidataEndEdges;
	for (int i = 0; i < handler->m_allParts.size(); i++)
	{
		for (int j = 0; j < handler->m_allParts[i]->edges.size(); j++)
		{
			Edge e = handler->m_allParts[i]->edges[j];
			if (e.m_isVisible)
			{
				Edge e1 = e;
				e1.m_point_edge_dis = 100;
				for (int k = 0; k < e.m_points.size(); k++)
				{
					if (e1.m_point_edge_dis > Utils::Calculate2DTwoPointDistance(endPoint, e.m_points[k]))
					{
						e1.m_point_edge_dis = Utils::Calculate2DTwoPointDistance(endPoint, e.m_points[k]);//m_point_edge_dis初始值-1
					}
				}
				if (e1.m_point_edge_dis < 30 && e1.m_point_edge_dis != -1)
				{
					candidataEndEdges.push_back(e1);
				}

			}
		}
	}
	//对候选边按距离m_point_edge_dis从小到大进行排序
	//start
	if (candidataStartEdges.size() > 0)
	{
		for (int i = 0; i < candidataStartEdges.size() - 1; i++)
		{
			Edge temp_e = candidataStartEdges[i];
			for (int j = candidataStartEdges.size() - 1; j > i; j--)
			{
				if (temp_e.m_point_edge_dis > candidataStartEdges[j].m_point_edge_dis)
				{
					candidataStartEdges[i] = candidataStartEdges[j];
					candidataStartEdges[j] = temp_e;
					temp_e = candidataStartEdges[i];
				}
			}
		}
	}
	//end
	if (candidataEndEdges.size() > 0)
	{
		for (int i = 0; i < candidataEndEdges.size() - 1; i++)
		{
			Edge temp_e = candidataEndEdges[i];
			for (int j = candidataEndEdges.size() - 1; j > i; j--)
			{
				if (temp_e.m_point_edge_dis > candidataEndEdges[j].m_point_edge_dis)
				{
					candidataEndEdges[i] = candidataEndEdges[j];
					candidataEndEdges[j] = temp_e;
					temp_e = candidataEndEdges[i];
				}
			}
		}
	}
	//根据方向选出最终两条边
	cv::Vec2d dir1, dir2,dir3;
	dir1 = Utils::Nomalize(endPoint - startPoint);
	//dir2 = e.m_dir2D;
	bool satisfyingRelation=false;
	Edge startEdge, endEdge;
	for (int i = 0; i < candidataStartEdges.size();i++)
	{
		for (int j = 0; j < candidataEndEdges.size();j++)
		{
			startEdge = candidataStartEdges[i];
			endEdge = candidataEndEdges[j];
			//Utils::Nomalize();
			dir2 = Utils::Nomalize(startEdge.m_dir2D);
			dir3 = Utils::Nomalize(endEdge.m_dir2D);
			double dir6, dir7;
			if ((abs(dir1.dot(dir2))>0.9 || isnan(abs(dir1.dot(dir2)))) && (abs(dir1.dot(dir3))>0.9 || isnan(abs(dir1.dot(dir3)))))
			{
				satisfyingRelation = true;
				 dir6 = abs(dir1.dot(dir2));
				 dir7 = abs(dir1.dot(dir3));
				break;
			}
		}
		if (satisfyingRelation)
		{
			break;
		}
	}
	if (!satisfyingRelation)
	{
		isAddRelaion = false;
	}
	//起点
	//Edge startEdge;
	//double dis = 100;
	//for (int i = 0; i < handler->m_allParts.size(); i++)
	//{
	//	for (int j = 0; j < handler->m_allParts[i]->edges.size(); j++)
	//	{
	//		Edge e = handler->m_allParts[i]->edges[j];
	//		if (e.m_isVisible)
	//		{
	//			for (int k = 0; k < e.m_points.size(); k++)
	//			{
	//				if (dis > Utils::Calculate2DTwoPointDistance(startPoint, e.m_points[k]))
	//				{
	//					startEdge = e;
	//					dis = Utils::Calculate2DTwoPointDistance(startPoint, e.m_points[k]);
	//				}
	//			}

	//		}
	//	}
	//}
	//if (dis > 30)
	//	isAddRelaion = false;
	////终点
	//Edge endEdge;
	//dis = 100;
	//for (int i = 0; i < handler->m_allParts.size(); i++)
	//{
	//	for (int j = 0; j < handler->m_allParts[i]->edges.size(); j++)
	//	{
	//		Edge e = handler->m_allParts[i]->edges[j];
	//		//
	//		cv::Vec2d dir1, dir2;
	//		dir1 = endPoint-startPoint;
	//		dir2 = e.m_dir2D;
	//		if (e.m_isVisible && (abs(dir1.dot(dir2))>0.7 || isnan(abs(dir1.dot(dir2)))))&& (abs(dir1.dot(dir2))>0.7 || isnan(abs(dir1.dot(dir2))))
	//		{
	//			for (int k = 0; k < e.m_points.size(); k++)
	//			{
	//				if (dis > Utils::Calculate2DTwoPointDistance(endPoint, e.m_points[k]))
	//				{
	//					endEdge = e;
	//					dis = Utils::Calculate2DTwoPointDistance(endPoint, e.m_points[k]);
	//				}
	//			}

	//		}
	//	}
	//}
	//if (dis > 30)
	//	isAddRelaion = false;
	//////////////////////////////////////////////////////////////////////////
	if (startEdge.m_shapeIndex == endEdge.m_shapeIndex)
		isAddRelaion = false;
	if (abs(startEdge.m_dir3D.dot(endEdge.m_dir3D)) > 0.9 || isnan(abs(startEdge.m_dir3D.dot(endEdge.m_dir3D))))
	{
	}
	else
	{
		double dir5 = abs(startEdge.m_dir3D.dot(endEdge.m_dir3D));
		isAddRelaion = false;
	}
	//距离限制
	float distance = handler->ComputEdgeToLineDis(startEdge, endEdge);//计算两条边的距离
	if (distance > 50)
		isAddRelaion = false;
	if (isAddRelaion)
	{
		ShapeRelation shapeRelation;
		shapeRelation.edges[0] = startEdge;
		shapeRelation.edges[1] = endEdge;
		RecordNo recordNo;
		double dis1 = Utils::Calculate2DTwoPointDistance(startEdge.m_projectPt2D[0], endEdge.m_projectPt2D[0]);
		double dis2 = Utils::Calculate2DTwoPointDistance(startEdge.m_projectPt2D[0], endEdge.m_projectPt2D[1]);
		double dis3 = Utils::Calculate2DTwoPointDistance(startEdge.m_projectPt2D[1], endEdge.m_projectPt2D[0]);
		double dis4 = Utils::Calculate2DTwoPointDistance(startEdge.m_projectPt2D[1], endEdge.m_projectPt2D[1]);
		double min = dis1;
		cv::Vec2d point1, point2;
		if (dis2 < min)
			min = dis2;
		if (dis3 < min)
			min = dis3;
		if (dis4 < min)
			min = dis4;
		if (dis1 == min)
		{
			point1 = startEdge.m_projectPt2D[0];
			point2 = endEdge.m_projectPt2D[0];
		}
		if (dis2 == min)
		{
			point1 = startEdge.m_projectPt2D[0];
			point2 = endEdge.m_projectPt2D[1];
		}
		if (dis3 == min)
		{
			point1 = startEdge.m_projectPt2D[1];
			point2 = endEdge.m_projectPt2D[0];
		}
		if (dis4 == min)
		{
			point1 = startEdge.m_projectPt2D[1];
			point2 = endEdge.m_projectPt2D[1];
		}
		QPointF point, p1, p2;
		p1.setX(point1[0]);
		p1.setY(point1[1]);
		p2.setX(point2[0]);
		p2.setY(point2[1]);
		for (int s = 0; s < handler->m_allParts.size(); s++)
		{
			for (int v = 0; v < handler->m_allParts[s]->edges.size(); v++)
			{
				point.setX(handler->m_allParts[s]->edges[v].m_projectPt2D[0][0]);
				point.setY(handler->m_allParts[s]->edges[v].m_projectPt2D[0][1]);
				if (point == p1)
				{
					recordNo.edge_no1 = v;
					recordNo.shape_no1 = s;
					recordNo.vertices_no1 = 0;
					break;
				}
				point.setX(handler->m_allParts[s]->edges[v].m_projectPt2D[1][0]);
				point.setY(handler->m_allParts[s]->edges[v].m_projectPt2D[1][1]);
				if (point == p1)
				{
					recordNo.edge_no1 = v;
					recordNo.shape_no1 = s;
					recordNo.vertices_no1 = 1;
					break;
				}
			}
			for (int v = 0; v < handler->m_allParts[s]->edges.size(); v++)
			{
				point.setX(handler->m_allParts[s]->edges[v].m_projectPt2D[0][0]);
				point.setY(handler->m_allParts[s]->edges[v].m_projectPt2D[0][1]);
				if (point == p2)
				{
					recordNo.edge_no2 = v;
					recordNo.shape_no2 = s;
					recordNo.vertices_no2 = 0;
					break;
				}
				point.setX(handler->m_allParts[s]->edges[v].m_projectPt2D[1][0]);
				point.setY(handler->m_allParts[s]->edges[v].m_projectPt2D[1][1]);
				if (point == p2)
				{
					recordNo.edge_no2 = v;
					recordNo.shape_no2 = s;
					recordNo.vertices_no2 = 1;
					break;
				}
			}
		}
		shapeRelation.recordNo = recordNo;
		//如果add里面有这个删除的关系，则删除
		for (vector<ShapeRelation>::iterator it = m_addRelations.begin(); it != m_addRelations.end();)
		{
			if (TwoRelaionIsEqual(*it, shapeRelation))
			{
				it = m_addRelations.erase(it);
				break;
			}
			else
			{
				it++;
			}
		}
		//把添加的关系放到add里面
		m_addRelations.push_back(shapeRelation);
		m_editRelationType = 1;//添加关系
		//如果delete里面有这个添加的关系，则删除

		for (vector<ShapeRelation>::iterator it = m_deleteRelations.begin(); it != m_deleteRelations.end();)
		{
			if (TwoRelaionIsEqual(*it, shapeRelation))
			{
				it = m_deleteRelations.erase(it);
				break;
			}
			else
			{
				it++;
			}
		}
	}
	else{
		//删除关系
		cv::Vec3d pointA, pointB, pointC, pointD;
		pointC[0] = m_relationStrokeAL.front()[0];
		pointC[1] = m_relationStrokeAL.front()[1];
		pointC[2] = 0;
		pointD[0] = m_relationStrokeAL.back()[0];
		pointD[1] = m_relationStrokeAL.back()[1];
		pointD[2] = 0;

		for (int i = 0; i < m_recordShowRelaion.size(); i++)
		{
			RecordNo no = m_recordShowRelaion[i].recordNo;
			if (no.shape_no1 < handler->m_allParts.size() && no.shape_no2 < handler->m_allParts.size() && no.edge_no1 < 12 && no.edge_no2 < 12)//防止越界
			{
				pointA[0] = handler->m_allParts[no.shape_no1]->edges[no.edge_no1].m_projectPt2D[no.vertices_no1][0];
				pointA[1] = handler->m_allParts[no.shape_no1]->edges[no.edge_no1].m_projectPt2D[no.vertices_no1][1];
				pointA[2] = 0;
				pointB[0] = (handler->m_allParts[no.shape_no2]->edges[no.edge_no2].m_projectPt2D[no.vertices_no2][0]);
				pointB[1] = (handler->m_allParts[no.shape_no2]->edges[no.edge_no2].m_projectPt2D[no.vertices_no2][1]);
				pointB[2] = 0;
				cv::Vec3d pointAB = pointB - pointA;
				cv::Vec3d pointCD = pointD - pointC;
				cv::Vec3d pointAC = pointC - pointA;
				cv::Vec3d pointAD = pointD - pointA;
				cv::Vec3d pointCA = pointA - pointC;
				cv::Vec3d pointCB = pointB - pointC;
				//
				cv::Vec3d pointABAC = pointAB.cross(pointAC);
				cv::Vec3d pointABAD = pointAB.cross(pointAD);
				//
				cv::Vec3d pointCDCA = pointCD.cross(pointCA);
				cv::Vec3d pointCDCB = pointCD.cross(pointCB);
				if (pointABAC.dot(pointABAD) == 0 || pointABAC.dot(pointABAD) < 0)
				{
					//std::cout <<"===========pointABAC"<<pointABAC.dot(pointABAD) << "\n";
					if (pointCDCA.dot(pointCDCB) == 0 || pointCDCA.dot(pointCDCB) < 0)
					{
						//std::cout << "===========pointCDCA" << pointCDCA.dot(pointCDCB) << "\n";
						//it = m_editRelations.erase(it);
						//把添加的关系放到add里面
						m_deleteRelations.push_back(m_recordShowRelaion[i]);
						m_editRelationType = 2;//删除关系
						//如果add里面有这个删除的关系，则删除
						for (vector<ShapeRelation>::iterator it = m_addRelations.begin(); it != m_addRelations.end();)
						{
							if (TwoRelaionIsEqual(*it, m_recordShowRelaion[i]))
							{
								it = m_addRelations.erase(it);
								break;
							}
							else
							{
								it++;
							}
						}
					}
				}
			}
		}
	}
	GetFinalOptRelaions();
	m_recordRelations.clear();
	for (int i = 0; i < m_optRelations.size(); i++)
	{
		m_recordRelations.push_back(m_optRelations[i]);
	}
	RemoveErrorRelaions(*handler);
	emit UpdateRelationsSender();
}

void CDisplayWidget::GetFinalOptRelaions()
{
	if (m_optRelations.empty())
		return;
	if (m_addRelations.empty() && m_deleteRelations.empty())
		return;
	//更新add
	for (int i = 0; i < m_addRelations.size(); i++)
	{
		m_addRelations[i].edges[0] = m_shapeHandler_h->m_allParts[m_addRelations[i].edges[0].m_shapeIndex]->edges[m_addRelations[i].edges[0].m_shapeEdgeNo];
		m_addRelations[i].edges[1] = m_shapeHandler_h->m_allParts[m_addRelations[i].edges[1].m_shapeIndex]->edges[m_addRelations[i].edges[1].m_shapeEdgeNo];
		ShapeRelation shapeRelation = m_addRelations[i];
		Edge startEdge = shapeRelation.edges[0];
		Edge endEdge=shapeRelation.edges[1];
		RecordNo recordNo;
		double dis1 = Utils::Calculate2DTwoPointDistance(startEdge.m_projectPt2D[0], endEdge.m_projectPt2D[0]);
		double dis2 = Utils::Calculate2DTwoPointDistance(startEdge.m_projectPt2D[0], endEdge.m_projectPt2D[1]);
		double dis3 = Utils::Calculate2DTwoPointDistance(startEdge.m_projectPt2D[1], endEdge.m_projectPt2D[0]);
		double dis4 = Utils::Calculate2DTwoPointDistance(startEdge.m_projectPt2D[1], endEdge.m_projectPt2D[1]);
		double min = dis1;
		cv::Vec2d point1, point2;
		if (dis2 < min)
			min = dis2;
		if (dis3 < min)
			min = dis3;
		if (dis4 < min)
			min = dis4;
		if (dis1 == min)
		{
			point1 = startEdge.m_projectPt2D[0];
			point2 = endEdge.m_projectPt2D[0];
		}
		if (dis2 == min)
		{
			point1 = startEdge.m_projectPt2D[0];
			point2 = endEdge.m_projectPt2D[1];
		}
		if (dis3 == min)
		{
			point1 = startEdge.m_projectPt2D[1];
			point2 = endEdge.m_projectPt2D[0];
		}
		if (dis4 == min)
		{
			point1 = startEdge.m_projectPt2D[1];
			point2 = endEdge.m_projectPt2D[1];
		}
		QPointF point, p1, p2;
		p1.setX(point1[0]);
		p1.setY(point1[1]);
		p2.setX(point2[0]);
		p2.setY(point2[1]);
		for (int s = 0; s < m_shapeHandler_h->m_allParts.size(); s++)
		{
			for (int v = 0; v < m_shapeHandler_h->m_allParts[s]->edges.size(); v++)
			{
				point.setX(m_shapeHandler_h->m_allParts[s]->edges[v].m_projectPt2D[0][0]);
				point.setY(m_shapeHandler_h->m_allParts[s]->edges[v].m_projectPt2D[0][1]);
				if (point == p1)
				{
					recordNo.edge_no1 = v;
					recordNo.shape_no1 = s;
					recordNo.vertices_no1 = 0;
					break;
				}
				point.setX(m_shapeHandler_h->m_allParts[s]->edges[v].m_projectPt2D[1][0]);
				point.setY(m_shapeHandler_h->m_allParts[s]->edges[v].m_projectPt2D[1][1]);
				if (point == p1)
				{
					recordNo.edge_no1 = v;
					recordNo.shape_no1 = s;
					recordNo.vertices_no1 = 1;
					break;
				}
			}
			for (int v = 0; v < m_shapeHandler_h->m_allParts[s]->edges.size(); v++)
			{
				point.setX(m_shapeHandler_h->m_allParts[s]->edges[v].m_projectPt2D[0][0]);
				point.setY(m_shapeHandler_h->m_allParts[s]->edges[v].m_projectPt2D[0][1]);
				if (point == p2)
				{
					recordNo.edge_no2 = v;
					recordNo.shape_no2 = s;
					recordNo.vertices_no2 = 0;
					break;
				}
				point.setX(m_shapeHandler_h->m_allParts[s]->edges[v].m_projectPt2D[1][0]);
				point.setY(m_shapeHandler_h->m_allParts[s]->edges[v].m_projectPt2D[1][1]);
				if (point == p2)
				{
					recordNo.edge_no2 = v;
					recordNo.shape_no2 = s;
					recordNo.vertices_no2 = 1;
					break;
				}
			}
		}
		m_addRelations[i].recordNo = recordNo;
	}
	//更新delete
	for (int i = 0; i < m_deleteRelations.size(); i++)
	{
		m_deleteRelations[i].edges[0] = m_shapeHandler_h->m_allParts[m_deleteRelations[i].edges[0].m_shapeIndex]->edges[m_deleteRelations[i].edges[0].m_shapeEdgeNo];
		m_deleteRelations[i].edges[1] = m_shapeHandler_h->m_allParts[m_deleteRelations[i].edges[1].m_shapeIndex]->edges[m_deleteRelations[i].edges[1].m_shapeEdgeNo];
		ShapeRelation shapeRelation = m_deleteRelations[i];
		Edge startEdge = shapeRelation.edges[0];
		Edge endEdge = shapeRelation.edges[1];
		RecordNo recordNo;
		double dis1 = Utils::Calculate2DTwoPointDistance(startEdge.m_projectPt2D[0], endEdge.m_projectPt2D[0]);
		double dis2 = Utils::Calculate2DTwoPointDistance(startEdge.m_projectPt2D[0], endEdge.m_projectPt2D[1]);
		double dis3 = Utils::Calculate2DTwoPointDistance(startEdge.m_projectPt2D[1], endEdge.m_projectPt2D[0]);
		double dis4 = Utils::Calculate2DTwoPointDistance(startEdge.m_projectPt2D[1], endEdge.m_projectPt2D[1]);
		double min = dis1;
		cv::Vec2d point1, point2;
		if (dis2 < min)
			min = dis2;
		if (dis3 < min)
			min = dis3;
		if (dis4 < min)
			min = dis4;
		if (dis1 == min)
		{
			point1 = startEdge.m_projectPt2D[0];
			point2 = endEdge.m_projectPt2D[0];
		}
		if (dis2 == min)
		{
			point1 = startEdge.m_projectPt2D[0];
			point2 = endEdge.m_projectPt2D[1];
		}
		if (dis3 == min)
		{
			point1 = startEdge.m_projectPt2D[1];
			point2 = endEdge.m_projectPt2D[0];
		}
		if (dis4 == min)
		{
			point1 = startEdge.m_projectPt2D[1];
			point2 = endEdge.m_projectPt2D[1];
		}
		QPointF point, p1, p2;
		p1.setX(point1[0]);
		p1.setY(point1[1]);
		p2.setX(point2[0]);
		p2.setY(point2[1]);
		for (int s = 0; s < m_shapeHandler_h->m_allParts.size(); s++)
		{
			for (int v = 0; v < m_shapeHandler_h->m_allParts[s]->edges.size(); v++)
			{
				point.setX(m_shapeHandler_h->m_allParts[s]->edges[v].m_projectPt2D[0][0]);
				point.setY(m_shapeHandler_h->m_allParts[s]->edges[v].m_projectPt2D[0][1]);
				if (point == p1)
				{
					recordNo.edge_no1 = v;
					recordNo.shape_no1 = s;
					recordNo.vertices_no1 = 0;
					break;
				}
				point.setX(m_shapeHandler_h->m_allParts[s]->edges[v].m_projectPt2D[1][0]);
				point.setY(m_shapeHandler_h->m_allParts[s]->edges[v].m_projectPt2D[1][1]);
				if (point == p1)
				{
					recordNo.edge_no1 = v;
					recordNo.shape_no1 = s;
					recordNo.vertices_no1 = 1;
					break;
				}
			}
			for (int v = 0; v < m_shapeHandler_h->m_allParts[s]->edges.size(); v++)
			{
				point.setX(m_shapeHandler_h->m_allParts[s]->edges[v].m_projectPt2D[0][0]);
				point.setY(m_shapeHandler_h->m_allParts[s]->edges[v].m_projectPt2D[0][1]);
				if (point == p2)
				{
					recordNo.edge_no2 = v;
					recordNo.shape_no2 = s;
					recordNo.vertices_no2 = 0;
					break;
				}
				point.setX(m_shapeHandler_h->m_allParts[s]->edges[v].m_projectPt2D[1][0]);
				point.setY(m_shapeHandler_h->m_allParts[s]->edges[v].m_projectPt2D[1][1]);
				if (point == p2)
				{
					recordNo.edge_no2 = v;
					recordNo.shape_no2 = s;
					recordNo.vertices_no2 = 1;
					break;
				}
			}
		}
		m_deleteRelations[i].recordNo = recordNo;
	}
	//更新add和delete后，如果该关系不满足则删除
	//add
	for (vector<ShapeRelation>::iterator it = m_addRelations.begin(); it != m_addRelations.end();)
	{
		if (abs(it->edges[0].m_dir3D.dot(it->edges[1].m_dir3D))>0.9 || isnan(abs(it->edges[0].m_dir3D.dot(it->edges[1].m_dir3D))))
		{
			float dis = m_shapeHandler_h->ComputEdgeToLineDis(it->edges[0], it->edges[1]);//计算两条边的距离
			if (dis>50)
			{
				it = m_addRelations.erase(it);
			}
			else
			{
				it++;
			}
		}
		else
		{
			it=m_addRelations.erase(it);
		}
	}
	//delete
	//for (vector<ShapeRelation>::iterator it = m_deleteRelations.begin(); it != m_deleteRelations.end();)
	//{
	//	if (abs(it->edges[0].m_dir3D.dot(it->edges[1].m_dir3D)) > 0.95 || isnan(abs(it->edges[0].m_dir3D.dot(it->edges[1].m_dir3D))))
	//	{
	//		float dis = m_shapeHandler_h->ComputEdgeToLineDis(it->edges[0], it->edges[1]);//计算两条边的距离
	//		if ( dis > 52)
	//		{
	//			it = m_deleteRelations.erase(it);
	//		}
	//		else
	//		{
	//			it++;
	//		}
	//	}
	//	else
	//	{
	//		it = m_deleteRelations.erase(it);
	//	}
	//}
	//如果opt里面有add的元素则删除
	
	for (int j = 0; j < m_addRelations.size(); j++)
	{
		for (vector<ShapeRelation>::iterator it = m_optRelations.begin(); it != m_optRelations.end();)
		{
			if (TwoRelaionIsEqual(*it, m_addRelations[j]))
			{
				it = m_optRelations.erase(it);
			}
			else
			{
				it++;
			}
		}
	}
	//如果opt里面有delete的元素则删除
	for (int j = 0; j < m_deleteRelations.size(); j++)
	{
		for (vector<ShapeRelation>::iterator it = m_optRelations.begin(); it != m_optRelations.end();)
		{
			if (TwoRelaionIsEqual(*it, m_deleteRelations[j]))
			{
				it=m_optRelations.erase(it);
			}
			else
			{
				it++;
			}
		}
	}
	//把add里面的元素反向加到opt前面
	vector<ShapeRelation> relations;
	for (int i=m_addRelations.size()-1;i>-1; i--)
	{
		relations.push_back(m_addRelations[i]);
	}
	for (int i = 0; i < m_optRelations.size(); i++)
	{
		relations.push_back(m_optRelations[i]);
	}
	m_optRelations.clear();
	for (int i = 0; i < relations.size(); i++)
	{
		m_optRelations.push_back(relations[i]);
	}
}


void CDisplayWidget::init()
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//// Light0 is the default ambient light
	glEnable(GL_LIGHT0);
	//position
	//float pos[4] = { 1.0, 0.5, 1.0, 0.0 };
	
	//glLightfv(GL_LIGHT0, GL_POSITION, pos);
	// Directionnal 
	//glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION,qglviewer::Vec(0, 0, 1));
	//
	const GLfloat light_ambient[4] = { 0.8f, 0.2f, 0.2f, 1.0 };
	const GLfloat light_diffuse[4] = { 1.0, 0.4f, 0.4f, 1.0 };
	const GLfloat light_specular[4] = { 1.0, 0.0, 0.0, 1.0 };

	glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 3);
	glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 20);
	glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 0.5);
	glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 1);
	glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 1.5);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	//// Light1 is a spot light
	/*glEnable(GL_LIGHT1);
	const GLfloat light_ambient[4] = { 0.8f, 0.2f, 0.2f, 1.0 };
	const GLfloat light_diffuse[4] = { 1.0, 0.4f, 0.4f, 1.0 };
	const GLfloat light_specular[4] = { 1.0, 0.0, 0.0, 1.0 };

	glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 3.0);
	glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 20.0);
	glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 0.5);
	glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 1.0);
	glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 1.5);
	glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);*/

	// Light2 is a classical directionnal light
	//glEnable(GL_LIGHT2);
	//const GLfloat light_ambient2[4] = { 0.2f, 0.2f, 2.0, 1.0 };
	//const GLfloat light_diffuse2[4] = { 0.8f, 0.8f, 1.0, 1.0 };
	//const GLfloat light_specular2[4] = { 0.0, 0.0, 1.0, 1.0 };

	//glLightfv(GL_LIGHT2, GL_AMBIENT, light_ambient2);
	//glLightfv(GL_LIGHT2, GL_SPECULAR, light_specular2);
	//glLightfv(GL_LIGHT2, GL_DIFFUSE, light_diffuse2);

	
}

void CDisplayWidget::SetWriter(std::fstream* writerp)
{
	m_writerp = writerp;
}

void CDisplayWidget::SaveCrtPaintArea(char* imgPath)
{
	/*int imgWidth, imgHeight;
	imgWidth = this->width();
	imgHeight = this->height();
	QImage image(imgWidth, imgHeight, QImage::Format_ARGB32_Premultiplied);
	image.fill(QColor(0, 0, 0, 0));
	QPainter thisPainter;
	thisPainter.begin(&image);
	thisPainter.setRenderHint(QPainter::Antialiasing, true);
	thisPainter.setBackground(Qt::white);
	thisPainter.end();*/
	QPixmap pixmap = QPixmap::grabWindow(this->winId());
	QImage image = pixmap.toImage();
	image.save(imgPath);
}

void CDisplayWidget::FinishWrite()
{
	m_writerp = NULL;
}

void CDisplayWidget::SortRelaions()
{
	if (m_recordRelations.empty())
		return;
	//通过两条边的距离排序，用于关系好坏的排序
	for (int i = 0; i < m_recordRelations.size(); i++)
	{
		Edge e1, e2;
		e1 = m_recordRelations[i].edges[0];
		e2 = m_recordRelations[i].edges[1];
		double dis1 = m_shapeHandler_h->ComputEdgeToLineDis(e1, e2);
		double dis2 = m_shapeHandler_h->ComputEdgeToLineDis(e2, e1);
		m_recordRelations[i].dis = abs(dis1+dis2)/10;
	}
	//根据距离排序，距离越小关系越好
	ShapeRelation temp;
	for (int i = 0; i < m_recordRelations.size() - 1; i++)
	{
		temp = m_recordRelations[i];
		for (int j = m_recordRelations.size() - 1; j > i; j--)
		{
			if (temp.dis > m_recordRelations[j].dis)
			{
				m_recordRelations[i] = m_recordRelations[j];
				m_recordRelations[j] = temp;
				temp = m_recordRelations[i];
			}
		}
	}
}

void CDisplayWidget::GetSpacingRelations(std::vector<ShapeRelation> relation)
{
	m_SpacingRelations.clear();
	std::vector<std::vector<ShapeRelation>> shapeRelaionss;
	while (true)
	{
		if (relation.empty())
			break;
		std::vector<ShapeRelation> shapeRelaions;
		std::vector<ShapeRelation>::iterator ite = relation.begin();
		shapeRelaions.push_back(*ite);
		Edge line = (*ite).edges[0];
		//
		//SortRecordNo
		RecordNo no = ite->recordNo;
		cv::Vec2d point1, point2;
		point1[0] = (m_shapeHandler_h->m_allParts[no.shape_no1]->edges[no.edge_no1].m_projectPt2D[no.vertices_no1][0]);
		point1[1] = (m_shapeHandler_h->m_allParts[no.shape_no1]->edges[no.edge_no1].m_projectPt2D[no.vertices_no1][1]);
		point2[0] = (m_shapeHandler_h->m_allParts[no.shape_no2]->edges[no.edge_no2].m_projectPt2D[no.vertices_no2][0]);
		point2[1] = (m_shapeHandler_h->m_allParts[no.shape_no2]->edges[no.edge_no2].m_projectPt2D[no.vertices_no2][1]);
		cv::Vec2d dir = point2 - point1;
		//
		ite = relation.erase(ite);
		for (; ite != relation.end();)
		{
			Edge e = (*ite).edges[0];
			Edge e2 = (*ite).edges[1];
			CShapeHandler shapeHandler;
			double dis = shapeHandler.ComputEdgeToLineDis(line, e);
			if (dis < 1 || isnan(dis))
			{
				//
				RecordNo no1 = ite->recordNo;
				cv::Vec2d point3, point4;
				point3[0] = (m_shapeHandler_h->m_allParts[no1.shape_no1]->edges[no1.edge_no1].m_projectPt2D[no1.vertices_no1][0]);
				point3[1] = (m_shapeHandler_h->m_allParts[no1.shape_no1]->edges[no1.edge_no1].m_projectPt2D[no1.vertices_no1][1]);
				point4[0] = (m_shapeHandler_h->m_allParts[no1.shape_no2]->edges[no1.edge_no2].m_projectPt2D[no1.vertices_no2][0]);
				point4[1] = (m_shapeHandler_h->m_allParts[no1.shape_no2]->edges[no1.edge_no2].m_projectPt2D[no1.vertices_no2][1]);
				cv::Vec2d dir1 = point4 - point3;
				/*std::cout << "==============dir " << dir << "\n";
				std::cout << "==============dir1 " << dir1<<"\n";
				std::cout << "==============dot " << dir1.dot(dir) << "\n";*/
				//
				if (dir1.dot(dir)<0)
				{
					//std::cout << "==============方向相反\n"; 
					RecordNo result;
					result.shape_no1 = no1.shape_no2;
					result.shape_no2 = no1.shape_no1;
					result.edge_no1 = no1.edge_no2;
					result.edge_no2 = no1.edge_no1;
					result.vertices_no1 = no1.vertices_no2;
					result.vertices_no2 = no1.vertices_no1;
					ite->recordNo = result;

				}
				
				shapeRelaions.push_back(*ite);
				ite = relation.erase(ite);
			}
			else
			{
				ite++;
			}
		}
		shapeRelaionss.push_back(shapeRelaions);
	}

	//std::cout << "shapeRelaionss size is " << shapeRelaionss.size() << "\n";
	//m_SpacingRelations.clear();
	std::vector<std::vector<RecordNo>> srs;
	for (int i = 0; i < shapeRelaionss.size();i++)
	{
		std::vector<RecordNo> recordNos;
		for (int j = 0; j < shapeRelaionss[i].size();j++)
		{
			recordNos.push_back(shapeRelaionss[i][j].recordNo);
		}
		if (recordNos.size() > 1)
		{
			int flag = 0;
			for (int k = 0; k < srs.size(); k++)
			{
				if (TwoRecordNoisEqual(recordNos, srs[k]))
				{
					flag = 1;
					//取同一个shape中屏幕上y值最大的线
					if (srs[k].size()>0)
					{
						RecordNo no = srs[k][0];
						for (int p = 0; p < recordNos.size();p++)
						{
							RecordNo recordNo = recordNos[p];
							if (recordNo.shape_no1 == no.shape_no1)
							{
								double y1 = (m_shapeHandler_h->m_allParts[recordNo.shape_no1]->edges[recordNo.edge_no1].m_projectPt2D[recordNo.vertices_no1][1]);
								double y2 = (m_shapeHandler_h->m_allParts[no.shape_no1]->edges[no.edge_no1].m_projectPt2D[no.vertices_no1][1]);
								if (y1>y2)
								{
									srs.erase(srs.begin() + k);
									srs.push_back(recordNos);
								}
								break;
							}
							if (recordNo.shape_no2 == no.shape_no1)
							{
								double y1 = (m_shapeHandler_h->m_allParts[recordNo.shape_no2]->edges[recordNo.edge_no1].m_projectPt2D[recordNo.vertices_no1][1]);
								double y2 = (m_shapeHandler_h->m_allParts[no.shape_no1]->edges[no.edge_no1].m_projectPt2D[no.vertices_no1][1]);
								if (y1 > y2)
								{
									srs.erase(srs.begin() + k);
									srs.push_back(recordNos);
								}
								break;
							}
						}
					}
					break;
				}
			}
			if (flag == 0)
			{
				srs.push_back(recordNos);
			}
		}
	}
	//按照距离分类
	std::vector<std::vector<RecordNo>> spacingRelations = srs;
	srs.clear();
	//m_SpacingRelations.clear();
	for (int i = 0; i < spacingRelations.size();i++)
	{
		std::vector<RecordNo> spacingRelation = spacingRelations[i];
		while (true)
		{
			if (spacingRelation.empty())
				break;
			std::vector<RecordNo> spacingGroup;//一组
			std::vector<RecordNo>::iterator ite = spacingRelation.begin();
			spacingGroup.push_back(*ite);
			RecordNo disNo =*ite;
			cv::Vec2d point1, point2;
			point1[0] = (m_shapeHandler_h->m_allParts[disNo.shape_no1]->edges[disNo.edge_no1].m_projectPt2D[disNo.vertices_no1][0]);
			point1[1] = (m_shapeHandler_h->m_allParts[disNo.shape_no1]->edges[disNo.edge_no1].m_projectPt2D[disNo.vertices_no1][1]);
			point2[0] = (m_shapeHandler_h->m_allParts[disNo.shape_no2]->edges[disNo.edge_no2].m_projectPt2D[disNo.vertices_no2][0]);
			point2[1] = (m_shapeHandler_h->m_allParts[disNo.shape_no2]->edges[disNo.edge_no2].m_projectPt2D[disNo.vertices_no2][1]);

			double dis1 = Utils::Calculate2DTwoPointDistance(point1, point2);
			ite = spacingRelation.erase(ite);
			for (; ite != spacingRelation.end();)
			{
				disNo = *ite;
				cv::Vec2d point1, point2;
				point1[0] = (m_shapeHandler_h->m_allParts[disNo.shape_no1]->edges[disNo.edge_no1].m_projectPt2D[disNo.vertices_no1][0]);
				point1[1] = (m_shapeHandler_h->m_allParts[disNo.shape_no1]->edges[disNo.edge_no1].m_projectPt2D[disNo.vertices_no1][1]);
				point2[0] = (m_shapeHandler_h->m_allParts[disNo.shape_no2]->edges[disNo.edge_no2].m_projectPt2D[disNo.vertices_no2][0]);
				point2[1] = (m_shapeHandler_h->m_allParts[disNo.shape_no2]->edges[disNo.edge_no2].m_projectPt2D[disNo.vertices_no2][1]);
				double dis2 = Utils::Calculate2DTwoPointDistance(point1, point2);
				double twoEdgeDis=abs(dis1 - dis2);
				std::cout << " twoEdgeDis is " << twoEdgeDis << "\n";
				if (twoEdgeDis<80)
				{
					spacingGroup.push_back(*ite);
					ite = spacingRelation.erase(ite);
				}
				else
				{
					ite++;
				}
			}
			if (spacingGroup.size()>1)
				srs.push_back(spacingGroup);
		}
	}
	//////////////////////////////////////////////////////////////////////////
	std::vector<std::vector<RecordNo>> temp = srs;
	//srs.clear();
	//for (int i = 0; i < temp.size(); i++)
	//{
	//	std::vector<RecordNo> result;
	//	//
	//     result = SortRecordNo(temp[i], m_shapeHandler_h);
	//	 //result = RuleOrdering(result);
	//	 srs.push_back(result);
	//}
	//把srs添加到m_SpacingRelations上面
	for (int i = 0; i < srs.size();i++)
	{
		//如果m_SpacingRelations没有，则添加
		int k = 0;
		for (k = 0; k < m_SpacingRelations.size();k++)
		{
			if (TwoRecordNoisEqual(srs[i], m_SpacingRelations[k]))
			{
				break;
			}
		}
		if (k == m_SpacingRelations.size())
		m_SpacingRelations.push_back(srs[i]);
	}
	
}

std::vector<std::vector<RecordNo>> CDisplayWidget::GetSpacingRelations(std::vector<std::vector<ShapeRelation>> group)
{
	std::vector<std::vector<RecordNo>> recordNoGroup;
	if (group.empty())
		return recordNoGroup;
	for (int i = 0; i < group.size();i++)
	{
		if (!group[i].empty())
		{
			std::vector<ShapeRelation>::iterator ite = group[i].begin();
			Edge line = ite->edges[0];
			ite++;
			for (; ite != group[i].end();)
			{
				Edge e = (*ite).edges[0];
				Edge e2 = (*ite).edges[1];
				CShapeHandler shapeHandler;
				double dis = shapeHandler.ComputEdgeToLineDis(line, e);
				if (dis < 1 || isnan(dis))
				{
					ite++;
				}
				else
				{
					ite = group[i].erase(ite);//不满足的删除
				}
			}
		}
	}
	//把spacing放入group
	for (int i = 0; i < group.size();i++)
	{
		std::vector<RecordNo> g;
		for (int j = 0; j < group[i].size();j++)
		{
			g.push_back(group[i][j].recordNo);
		}
		recordNoGroup.push_back(g);
	}
	
    //////////////////////////////////////////////////////////////////////////
	std::vector<std::vector<RecordNo>> temp = recordNoGroup;
	recordNoGroup.clear();
	for (int i = 0; i < temp.size(); i++)
	{
		std::vector<RecordNo> result;
		result = SortRecordNo(temp[i], m_shapeHandler_h);
		recordNoGroup.push_back(result);
	}
	return recordNoGroup;
}

vector<ShapeRelation> CDisplayWidget::UpdateVecRelations(vector<ShapeRelation> availableRelations)
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
	return availableRelations;
}

vector<ShapeRelation> CDisplayWidget::UpdateVecRelations(vector<ShapeRelation> availableRelations, CShapeHandler* handler)
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
		activePart1 = handler->m_allParts[e1.m_shapeIndex];
		activePart2 = handler->m_allParts[e2.m_shapeIndex];
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
	return availableRelations;
}

std::vector<std::vector<ShapeRelation>> CDisplayWidget::UpdateVecRelations(std::vector<std::vector<ShapeRelation>> group)
{
	CShapePart* activePart1;
	CShapePart* activePart2;
	for (int i = 0; i < group.size();i++)
	{
		for (int j = 0; j < group[i].size();j++)
		{
			//ShapeRelation s=group[i][j];
			ShapeRelation shaper;
			Edge e1, e2, ee1, ee2;
			e1 = group[i][j].edges[0];
			e2 = group[i][j].edges[1];
			activePart1 = m_shapeHandler_h->m_allParts[e1.m_shapeIndex];
			activePart2 = m_shapeHandler_h->m_allParts[e2.m_shapeIndex];
			ee1 = activePart1->edges[e1.m_shapeEdgeNo];
			ee2 = activePart2->edges[e2.m_shapeEdgeNo];
			ee1.m_shapeIndex = e1.m_shapeIndex;
			ee2.m_shapeIndex = e2.m_shapeIndex;
			shaper.edges[0] = ee1;
			shaper.edges[1] = ee2;
			shaper.is2relations = group[i][j].is2relations;
			shaper.recordNo = group[i][j].recordNo;
			group[i][j]=shaper;
		}
	}
	return group;
}

bool CDisplayWidget::TwoRecordNoisEqual(std::vector<RecordNo> no1, std::vector<RecordNo> no2)
{
	if (no1.size() != no2.size())
		return false;
	set<int> sets1;
	for (int i = 0; i < no1.size();i++)
	{
		RecordNo recordNO = no1[i];
		sets1.insert(recordNO.shape_no1);
		sets1.insert(recordNO.shape_no2);
	}
	set<int> sets2;
	for (int i = 0; i < no2.size(); i++)
	{
		RecordNo recordNO = no2[i];
		sets2.insert(recordNO.shape_no1);
		sets2.insert(recordNO.shape_no2);
	}
	for (set<int>::iterator ite = sets1.begin(); ite != sets1.end();ite++)
	{
		if (sets2.count(*ite)==0)
		{
			return false;
		}
	}
	return true;
}

std::vector<RecordNo> CDisplayWidget::SortRecordNo(std::vector<RecordNo> recordNos, CShapeHandler* handler)
{
	if (recordNos.size() < 2)
		return recordNos;
	vector<RecordNo> result;
	RecordNo no = recordNos[0];
	result.push_back(no);
	cv::Vec2d point1, point2;
	point1[0] = (handler->m_allParts[no.shape_no1]->edges[no.edge_no1].m_projectPt2D[no.vertices_no1][0]);
	point1[1] = (handler->m_allParts[no.shape_no1]->edges[no.edge_no1].m_projectPt2D[no.vertices_no1][1]);
	point2[0] = (handler->m_allParts[no.shape_no2]->edges[no.edge_no2].m_projectPt2D[no.vertices_no2][0]);
	point2[1] = (handler->m_allParts[no.shape_no2]->edges[no.edge_no2].m_projectPt2D[no.vertices_no2][1]);
	cv::Vec2d dir = point2 - point1;
	for (int i = 1; i < recordNos.size();i++)
	{
		RecordNo recordNo = recordNos[i];
		recordNo = SwapRecordNo(recordNo, dir);
		result.push_back(recordNo);
		
	}
	
	return result;
}

RecordNo CDisplayWidget::SwapRecordNo(RecordNo no, cv::Vec2d dir)
{
	RecordNo result;
	cv::Vec2d point1, point2;
	point1[0] = (m_shapeHandler_h->m_allParts[no.shape_no1]->edges[no.edge_no1].m_projectPt2D[no.vertices_no1][0]);
	point1[1] = (m_shapeHandler_h->m_allParts[no.shape_no1]->edges[no.edge_no1].m_projectPt2D[no.vertices_no1][1]);
	point2[0] = (m_shapeHandler_h->m_allParts[no.shape_no2]->edges[no.edge_no2].m_projectPt2D[no.vertices_no2][0]);
	point2[1] = (m_shapeHandler_h->m_allParts[no.shape_no2]->edges[no.edge_no2].m_projectPt2D[no.vertices_no2][1]);
	
	dir = Utils::Nomalize(point2-point1);
	//std::cout << "======" << Utils::Nomalize(point2 - point1).dot(dir) << "\n";
	std::cout << "================ dir is " << dir << "\n";
	std::cout << "================ dir2 is " << point2 - point1 << "\n";
	if ((point2 - point1).dot(dir)<0)
	{
		result.shape_no1 = no.shape_no2;
		result.shape_no2 = no.shape_no1;
		result.edge_no1 = no.edge_no2;
		result.edge_no2 = no.edge_no1;
		result.vertices_no1 = no.vertices_no2;
		result.vertices_no2 = no.vertices_no1;
		return result;
	}
	else
	{
		std::cout << "方向bu相反\n";
		return no;
	}
}

std::vector<RecordNo> CDisplayWidget::RuleOrdering(std::vector<RecordNo> recordNos)
{
	if (recordNos.empty())
		return recordNos;
	std::vector<RecordNo> result;
	std::list<RecordNo> listRecordNo;
	std::vector<RecordNo> nos = recordNos;
	for (int i = recordNos.size()-1; i > -1; i--)
	{
		listRecordNo.push_back(recordNos[i]);
	}
	/*int no=-1;
	std::vector<int> sumNos;
	for (int i = 0; i < nos.size();i++)
	{
	RecordNo rNo = nos[i];
	sumNos.push_back(rNo.shape_no1);
	sumNos.push_back(rNo.shape_no2);
	}*/
	/*for (int i = 0; i < sumNos.size();i++)
	{
		for (int j = 0; j < sumNos.size();j++)
		{
			if (i!=j)
			{
				if (sumNos[i] == sumNos[j])
				{
					no = sumNos[i];
					break;
				}
			}
		}
	}
	if (no==-1)
	{
		return recordNos;
	}
	for (int i = 0; i < nos.size();i++)
	{
		RecordNo rNo = nos[i];
		if (rNo.shape_no1==no||rNo.shape_no2==no)
		{
			listRecordNo.push_back(rNo);
			nos.erase(nos.begin()+i);
			break;
		}
	}
	if (listRecordNo.empty())
		return result;
	while (!nos.empty())
	{
		std::vector<RecordNo>::iterator ite;
		for (ite = nos.begin(); ite != nos.end();)
		{
			RecordNo back = listRecordNo.back();
			RecordNo front = listRecordNo.front();
			if (back.shape_no1 == ite->shape_no1 || back.shape_no1 == ite->shape_no2 || back.shape_no2 == ite->shape_no1 || back.shape_no2 == ite->shape_no2)
			{
				listRecordNo.push_back(*ite);
				ite = nos.erase(ite);
				continue;
			}
			else if (front.shape_no1 == ite->shape_no1 || front.shape_no1 == ite->shape_no2 || front.shape_no2 == ite->shape_no1 || front.shape_no2 == ite->shape_no2)
			{
				listRecordNo.push_front(*ite);
				ite = nos.erase(ite);
				continue;
			}
			ite++;
		}

	}*/

	std::list<RecordNo>::iterator ite;
	result.clear();
	for (ite = listRecordNo.begin(); ite != listRecordNo.end();++ite)
	{
		result.push_back(*ite);
	}
	return result;
}

void CDisplayWidget::DrawSpacings(QPainter& painter)
{
	if (m_SpacingRelations.empty())
		return;
	
	QColor c[2];
	QPen thisPen;
	//int triangleSize = m_lineWidth * 3;
	QStringList m_colorNames[2];
	m_colorNames[0].push_back("darkgreen");
	m_colorNames[0].push_back("chocolate");
	m_colorNames[0].push_back("darkcyan");
	m_colorNames[0].push_back("crimson");
	//m_colorNames[0].push_back("darkolivergreen");
	//m_colorNames[0].push_back("darkslateblue");
	m_colorNames[0].push_back("darkviolet");

	m_colorNames[0].push_back("cadetblut");
	m_colorNames[0].push_back("chartreuse");
	m_colorNames[0].push_back("cornflowerblue");
	m_colorNames[0].push_back("crimson");
	m_colorNames[0].push_back("firebrick");
	m_colorNames[0].push_back("forestgreen");
	m_colorNames[0].push_back("brown");

	m_colorNames[0].push_back("darkblue");
	m_colorNames[0].push_back("darkgoldenrod");
	m_colorNames[0].push_back("darkmagenta");
	m_colorNames[0].push_back("darkorange");
	m_colorNames[0].push_back("darkred");

	//m_colorNames[1].push_back("indigo");
	m_colorNames[1].push_back("navy");
	m_colorNames[1].push_back("maroon");
	//m_colorNames[1].push_back("midnightblue");
	m_colorNames[1].push_back("olive");
	//m_colorNames[1].push_back("orange");
	//m_colorNames[1].push_back("peru");
	m_colorNames[1].push_back("purple");
	m_colorNames[1].push_back("red");

	m_colorNames[1].push_back("seagreen");
	//m_colorNames[1].push_back("slateblue");
	m_colorNames[1].push_back("springgreen");
	m_colorNames[1].push_back("teal");
	m_colorNames[1].push_back("tomato");
	m_colorNames[1].push_back("yellowgreen");
	m_colorNames[1].push_back("tan");
	m_colorNames[1].push_back("sandybrown");
	m_colorNames[1].push_back("gold");
	m_colorNames[1].push_back("royalblue");
	m_colorNames[1].push_back("green");
	QStringList colorNames = m_colorNames[0];
	for (int i = 0; i < m_SpacingRelations.size(); i++)
	{
		painter.setRenderHint(QPainter::Antialiasing, false);
		c[0] = QColor(colorNames[i % colorNames.size()]);
		c[1] = QColor(colorNames[i % colorNames.size()]);
		c[0].setAlpha(255);
		c[1].setAlpha(255);
		QPoint p[2];
		painter.save();
		painter.setRenderHint(QPainter::Antialiasing, false);
		thisPen.setStyle(Qt::DotLine);
		thisPen.setBrush(QBrush(c[0]));
		thisPen.setWidthF(2.0);
		painter.setPen(thisPen);
		for (int j = 0; j < m_SpacingRelations[i].size(); j++)
		{
			//std::cout << "================ draw painter is " << m_SpacingRelations[i].size() << "\n";
			RecordNo no = m_SpacingRelations[i][j];
			if (no.shape_no1 < m_shapeHandler_h->m_allParts.size() && no.shape_no2 < m_shapeHandler_h->m_allParts.size() && no.edge_no1 < 12 && no.edge_no2 < 12)//防止越界
			{
				QPointF point1, point2;
				point1.setX(m_shapeHandler_h->m_allParts[no.shape_no1]->edges[no.edge_no1].m_projectPt2D[no.vertices_no1][0]);
				point1.setY(m_shapeHandler_h->m_allParts[no.shape_no1]->edges[no.edge_no1].m_projectPt2D[no.vertices_no1][1]);
				point2.setX(m_shapeHandler_h->m_allParts[no.shape_no2]->edges[no.edge_no2].m_projectPt2D[no.vertices_no2][0]);
				point2.setY(m_shapeHandler_h->m_allParts[no.shape_no2]->edges[no.edge_no2].m_projectPt2D[no.vertices_no2][1]);
				//painter.drawLine(point1, point2);
				//
				cv::Vec2d v1, v2;
				v1 = cv::Vec2d(point1.x(), point1.y());
				v2 = cv::Vec2d(point2.x(), point2.y());
				cv::Vec2d dir = cv::Vec2d(point2.x(), point2.y()) - cv::Vec2d(point1.x(), point1.y());
				dir = Utils::Nomalize(dir);
				double m = dir[0];
				double n = dir[1];
				double dy = sqrt(m*m / (n*n + m*m));
				double dx = -(n*dy / m);
				dir[0] = dx;
				dir[1] = dy;
				cv::Vec2d v3, v4;
				v3 = v1 + 30 * dir;
				v4 = v2 + 30 * dir;
				if (v3[1]<v1[1])
				{
					v3 = v1 + -30 * dir;
					v4 = v2 + -30 * dir;
				}
				painter.drawLine(QPointF(v1[0], v1[1]), QPointF(v3[0], v3[1]));
				painter.drawLine(QPointF(v2[0], v2[1]), QPointF(v4[0], v4[1]));
				cv::Vec2d v5, v6;
				v5 = (v1 + v3) / 2;
				v6 = (v2 + v4) / 2;
				painter.drawLine(QPointF(v5[0],v5[1]), QPointF(v6[0],v6[1]));
				//
				/*cv::Vec2d v7, v8;
				v7 = v5 + 18 * Utils::Nomalize(v6 - v5);
				v8 = v6 + 18 * Utils::Nomalize(v5 - v6);
				cv::Vec2d v9, v10;
				v9 = v7 + 10 * dir;
				v10 = v7 - 10 * dir;
				cv::Vec2d v11, v12;
				v11 = v8 + 10 * dir;
				v12 = v8 - 10 * dir;
				QPoint triangle0[3], triangle1[3];
				triangle0[0].setX(v5[0]);
				triangle0[0].setY(v5[1]);
				triangle0[1].setX(v9[0]);
				triangle0[1].setY(v9[1]);
				triangle0[2].setX(v10[0]);
				triangle0[2].setY(v10[1]);

				triangle1[0].setX(v6[0]);
				triangle1[0].setY(v6[1]);
				triangle1[1].setX(v11[0]);
				triangle1[1].setY(v11[1]);
				triangle1[2].setX(v12[0]);
				triangle1[2].setY(v12[1]);
				painter.drawPolygon(triangle0, 3);
				painter.drawPolygon(triangle1, 3);*/
			}
		}
		painter.restore();
		painter.save();
		painter.setRenderHint(QPainter::Antialiasing, true);
		painter.setBrush(QBrush(c[1]));
		painter.setPen(Qt::NoPen);
		for (int j = 0; j < m_SpacingRelations[i].size(); j++)
		{
			//std::cout << "================ draw painter is " << m_SpacingRelations[i].size() << "\n";
			RecordNo no = m_SpacingRelations[i][j];
			if (no.shape_no1 < m_shapeHandler_h->m_allParts.size() && no.shape_no2 < m_shapeHandler_h->m_allParts.size() && no.edge_no1 < 12 && no.edge_no2 < 12)//防止越界
			{
				QPointF point1, point2;
				point1.setX(m_shapeHandler_h->m_allParts[no.shape_no1]->edges[no.edge_no1].m_projectPt2D[no.vertices_no1][0]);
				point1.setY(m_shapeHandler_h->m_allParts[no.shape_no1]->edges[no.edge_no1].m_projectPt2D[no.vertices_no1][1]);
				point2.setX(m_shapeHandler_h->m_allParts[no.shape_no2]->edges[no.edge_no2].m_projectPt2D[no.vertices_no2][0]);
				point2.setY(m_shapeHandler_h->m_allParts[no.shape_no2]->edges[no.edge_no2].m_projectPt2D[no.vertices_no2][1]);
				//painter.drawLine(point1, point2);
				//
				cv::Vec2d v1, v2;
				v1 = cv::Vec2d(point1.x(), point1.y());
				v2 = cv::Vec2d(point2.x(), point2.y());
				cv::Vec2d dir = cv::Vec2d(point2.x(), point2.y()) - cv::Vec2d(point1.x(), point1.y());
				dir = Utils::Nomalize(dir);
				double m = dir[0];
				double n = dir[1];
				double dy = sqrt(m*m / (n*n + m*m));
				double dx = -(n*dy / m);
				dir[0] = dx;
				dir[1] = dy;
				cv::Vec2d v3, v4;
				v3 = v1 + 30 * dir;
				v4 = v2 + 30 * dir;
				if (v3[1] < v1[1])
				{
					v3 = v1 + -30 * dir;
					v4 = v2 + -30 * dir;
				}
				painter.drawLine(QPointF(v1[0], v1[1]), QPointF(v3[0], v3[1]));
				painter.drawLine(QPointF(v2[0], v2[1]), QPointF(v4[0], v4[1]));
				cv::Vec2d v5, v6;
				v5 = (v1 + v3) / 2;
				v6 = (v2 + v4) / 2;
				painter.drawLine(QPointF(v5[0], v5[1]), QPointF(v6[0], v6[1]));
				//
				cv::Vec2d v7, v8;
				v7 = v5 + 12 * Utils::Nomalize(v6 - v5);
				v8 = v6 + 12 * Utils::Nomalize(v5 - v6);
				cv::Vec2d v9, v10;
				v9 = v7 + 7 * dir;
				v10 = v7 - 7 * dir;
				cv::Vec2d v11, v12;
				v11 = v8 + 7 * dir;
				v12 = v8 - 7 * dir;
				QPoint triangle0[3], triangle1[3];
				triangle0[0].setX(v5[0]);
				triangle0[0].setY(v5[1]);
				triangle0[1].setX(v9[0]);
				triangle0[1].setY(v9[1]);
				triangle0[2].setX(v10[0]);
				triangle0[2].setY(v10[1]);

				triangle1[0].setX(v6[0]);
				triangle1[0].setY(v6[1]);
				triangle1[1].setX(v11[0]);
				triangle1[1].setY(v11[1]);
				triangle1[2].setX(v12[0]);
				triangle1[2].setY(v12[1]);
				painter.drawPolygon(triangle0, 3);
				painter.drawPolygon(triangle1, 3);
			}
		}
		painter.restore();
	}
}

double CDisplayWidget::ComputeTwoEdgeDis(Edge e1, Edge e2)
{
	double dis1 = Utils::Calculate2DTwoPointDistance(e1.m_projectPt2D[0], e1.m_projectPt2D[1]);
	double dis2 = Utils::Calculate2DTwoPointDistance(e2.m_projectPt2D[0], e2.m_projectPt2D[1]);
	return abs(dis1-dis2);
}

void CDisplayWidget::ComputeSpacingRelations()
{
	CShapeHandler *handle;
	if (m_mouseOperateType == SC::RecSelect)
	{
		handle = &m_partShapeHandler_h;
	}
	else
	{
		handle = m_shapeHandler_h;
	}
	if (m_optRelations.size() == 0)
		return;
	if (handle == NULL)
		return;
	if (handle->m_allParts.size() < 2)
		return;
	std::vector<ShapeRelation> clone_optRelations=m_optRelations;
	CShapeHandler clone_handler = *m_shapeHandler_h;
	int manipulatedFrameIndex = -1;
	for (int i = 0; i < m_shapeHandler_h->m_allParts.size(); i++)
	{
		if (m_shapeHandler_h->m_allParts[i]->m_frame == manipulatedFrame())
		{
			manipulatedFrameIndex = i;
			break;
		}
	}
	//选出正确的关系
	for (int i = 0; i < m_optRelations.size(); i++)
	{
		m_optRelations[i].is2relations = false;
	}
	for (int i = 0; i < m_optRelations.size(); i++)
	{
		ShapeRelation shapeRelation1 = m_optRelations[i];
		for (int j = 0; j < i; j++)
		{
			ShapeRelation shapeRelation2 = m_optRelations[j];
			if (shapeRelation2.edges[0].m_shapeIndex == shapeRelation1.edges[0].m_shapeIndex)
			{
				if (shapeRelation2.edges[1].m_shapeIndex == shapeRelation1.edges[1].m_shapeIndex)
				{
					double a = abs(shapeRelation1.edges[0].m_rotateNo - shapeRelation2.edges[0].m_rotateNo);
					double b = abs(shapeRelation1.edges[1].m_rotateNo - shapeRelation2.edges[1].m_rotateNo);
					if (a - b == 0)
					{
						m_optRelations[i].is2relations = true;
						//relations[i - 1].is2relations = true;
					}
					continue;
				}
			}
			if (shapeRelation2.edges[0].m_shapeIndex == shapeRelation1.edges[1].m_shapeIndex)
			{
				if (shapeRelation2.edges[1].m_shapeIndex == shapeRelation1.edges[0].m_shapeIndex)
				{
					double a = abs(shapeRelation1.edges[0].m_rotateNo - shapeRelation2.edges[1].m_rotateNo);
					double b = abs(shapeRelation1.edges[1].m_rotateNo - shapeRelation2.edges[0].m_rotateNo);
					if (a - b == 0)
					{
						m_optRelations[i].is2relations = true;
					}
					//relations[i].is2relations = true;
					continue;
				}
			}
		}

	}
	vector<bool> floatings;//记录所有shape的悬浮状态
	for (int i = 0; i < handle->m_allParts.size(); i++)
	{
		floatings.push_back(handle->m_allParts[i]->m_isFloating);
	}
	int k = 0;
	for (k = 0; k < m_optRelations.size(); k++)
	{
		if (!floatings[m_optRelations[k].edges[0].m_shapeIndex]) break;
		if (!floatings[m_optRelations[k].edges[1].m_shapeIndex]) break;
	}
	if (k == m_optRelations.size())
	{
		floatings[0] = false;
		m_optRelations.clear();
		return;
	}
	handle->FixAllModels(camera());
	CLayoutRefiner layoutRefiner;
	layoutRefiner.SetData(handle);
	//
	layoutRefiner.ExecuteAlignmentOptimizing(m_optRelations, this->camera(), &floatings);
	FixModelsByFloatIndex(floatings, handle);

	m_shapeHandler_h->ComputeEdgeAndNormal(camera());
	m_optRelations = UpdateVecRelations(m_optRelations);
	GetSpacingRelations(m_optRelations);
	m_addSpacingRelationsGroup.push_back(m_addSpacingRelations);
	std::vector<std::vector<ShapeRelation>> arg = UpdateVecRelations(m_addSpacingRelationsGroup);
	m_addSpacingRelationsGroup.pop_back();
	std::vector<std::vector<RecordNo>> recordNoGroup=GetSpacingRelations(arg);
	GetFinalSpacingRelaions(recordNoGroup);
	
	CLayoutRefiner layoutRefiner1;
	layoutRefiner1.SetData(handle);
	layoutRefiner1.ExecuteSpacingOptimizing(m_optRelations, this->camera(), m_SpacingRelations);
	m_optRelations=clone_optRelations;
	m_shapeHandler_h->DestoryAllParts();
	for (int i = 0; i < clone_handler.m_allParts.size(); i++)
	{
		CShapePart shapePart = *clone_handler.m_allParts[i];
		m_shapeHandler_h->AddNewPart(shapePart);
	}
	if (manipulatedFrameIndex!=-1)
	{
		if (manipulatedFrameIndex < m_shapeHandler_h->m_allParts.size() && manipulatedFrameIndex>-1)
		{
			setManipulatedFrame(m_shapeHandler_h->m_allParts[manipulatedFrameIndex]->m_frame);
		}
	}
	//repaint();
}

void CDisplayWidget::DrawSpacingStroke(QPainter& painter)
{
	if (m_spacingStrokeAL.size() == 0)
		return;
	painter.save();
	painter.setRenderHint(QPainter::Antialiasing, true);
	QColor c(200, 0, 0, 200);
	QPen thisPen(QBrush(c), 5, Qt::SolidLine, Qt::RoundCap);
	thisPen.setJoinStyle(Qt::RoundJoin);
	painter.setPen(thisPen);
	QPainterPath path;
	path.moveTo(m_spacingStrokeAL[0][0], m_spacingStrokeAL[0][1]);
	for (int i = 1; i < m_spacingStrokeAL.size(); ++i)
	{
		path.lineTo(m_spacingStrokeAL[i][0], m_spacingStrokeAL[i][1]);
		path.moveTo(m_spacingStrokeAL[i][0], m_spacingStrokeAL[i][1]);
	}
	painter.drawPath(path);
	painter.restore();
}

void CDisplayWidget::RemoveSpacingRelations()
{
	if (m_spacingStrokeAL.size() < 2)
		return;
	//删除关系
	cv::Vec3d pointA, pointB, pointC, pointD;
	pointC[0] = m_spacingStrokeAL.front()[0];
	pointC[1] = m_spacingStrokeAL.front()[1];
	pointC[2] = 0;
	pointD[0] = m_spacingStrokeAL.back()[0];
	pointD[1] = m_spacingStrokeAL.back()[1];
	pointD[2] = 0;
	for (int i = 0; i < m_SpacingRelations.size();i++)
	{
		std::vector<RecordNo>::iterator ite = m_SpacingRelations[i].begin();
		for (; ite != m_SpacingRelations[i].end();)
		{
			RecordNo no = *ite;
			if (no.shape_no1 < m_shapeHandler_h->m_allParts.size() && no.shape_no2 < m_shapeHandler_h->m_allParts.size() && no.edge_no1 < 12 && no.edge_no2 < 12)//防止越界
			{
				pointA[0] = m_shapeHandler_h->m_allParts[no.shape_no1]->edges[no.edge_no1].m_projectPt2D[no.vertices_no1][0];
				pointA[1] = m_shapeHandler_h->m_allParts[no.shape_no1]->edges[no.edge_no1].m_projectPt2D[no.vertices_no1][1];
				pointA[2] = 0;
				pointB[0] = (m_shapeHandler_h->m_allParts[no.shape_no2]->edges[no.edge_no2].m_projectPt2D[no.vertices_no2][0]);
				pointB[1] = (m_shapeHandler_h->m_allParts[no.shape_no2]->edges[no.edge_no2].m_projectPt2D[no.vertices_no2][1]);
				pointB[2] = 0;
				//
				cv::Vec2d v1, v2;
				v1 = cv::Vec2d(pointA[0], pointA[1]);
				v2 = cv::Vec2d(pointB[0], pointB[1]);
				cv::Vec2d dir = cv::Vec2d(v2[0], v2[1]) - cv::Vec2d(v1[0],v1[1]);
				dir = Utils::Nomalize(dir);
				double m = dir[0];
				double n = dir[1];
				double dy = sqrt(m*m / (n*n + m*m));
				double dx = -(n*dy / m);
				dir[0] = dx;
				dir[1] = dy;
				cv::Vec2d v3, v4;
				v3 = v1 + 30 * dir;
				v4 = v2 + 30 * dir;
				if (v3[1] < v1[1])
				{
					v3 = v1 + -30 * dir;
					v4 = v2 + -30 * dir;
				}
				cv::Vec2d v5, v6;
				v5 = (v1 + v3) / 2;
				v6 = (v2 + v4) / 2;
				//
				pointA[0] = v5[0]; pointA[1] = v5[1];
				pointB[0] = v6[0]; pointB[1] = v6[1];
				cv::Vec3d pointAB = pointB - pointA;
				cv::Vec3d pointCD = pointD - pointC;
				cv::Vec3d pointAC = pointC - pointA;
				cv::Vec3d pointAD = pointD - pointA;
				cv::Vec3d pointCA = pointA - pointC;
				cv::Vec3d pointCB = pointB - pointC;

				cv::Vec3d pointABAC = pointAB.cross(pointAC);
				cv::Vec3d pointABAD = pointAB.cross(pointAD);

				cv::Vec3d pointCDCA = pointCD.cross(pointCA);
				cv::Vec3d pointCDCB = pointCD.cross(pointCB);
				int loopFlag = 0;
				if (pointABAC.dot(pointABAD) == 0 || pointABAC.dot(pointABAD) < 0)
				{
					if (pointCDCA.dot(pointCDCB) == 0 || pointCDCA.dot(pointCDCB) < 0)
					{
						loopFlag = 1;
						int flag = 0;
						for (int k = 0; k < m_deleteSpacingRelations.size(); k++)
						{
							if (TwoSpacingRelaionIsEqual(m_deleteSpacingRelations[k], no))
							{
								flag = 1;
								break;
							}
						}
						if (flag == 0)
						m_deleteSpacingRelations.push_back(no);
						
						//如果add里面有这个删除的关系，则删除
						for (vector<ShapeRelation>::iterator it = m_addSpacingRelations.begin(); it != m_addSpacingRelations.end();)
							{
								if (TwoSpacingRelaionIsEqual(it->recordNo, no))
								{
									it = m_addSpacingRelations.erase(it);
									break;
								}
								else
								{
									it++;
								}
							}
						if (m_addSpacingRelations.size()==1)
						   {
							   m_addSpacingRelations.clear();
						   }
						//如果group里面有这个删除的关系，则删除
						for (int m = 0; m < m_addSpacingRelationsGroup.size();m++)
						{
							for (vector<ShapeRelation>::iterator it = m_addSpacingRelationsGroup[m].begin(); it != m_addSpacingRelationsGroup[m].end();)
							{
								if (TwoSpacingRelaionIsEqual(it->recordNo, no))
								{
									it = m_addSpacingRelationsGroup[m].erase(it);
									break;
								}
								else
								{
									it++;
								}
							}
							//组内剩余一个则删除
							if (m_addSpacingRelationsGroup[m].size()==1)
							{
								m_addSpacingRelationsGroup.erase(m_addSpacingRelationsGroup.begin() + m);
								break;
							}
						}

						
					}
				}
				if (loopFlag==0)
				{
					ite++;
				}
				else
				{
					ite = m_SpacingRelations[i].erase(ite);
					if (m_SpacingRelations[i].size()==1)
					{
						int flag = 0;
						for (int k = 0; k < m_deleteSpacingRelations.size(); k++)
						{
							if (TwoSpacingRelaionIsEqual(m_deleteSpacingRelations[k], m_SpacingRelations[i][0]))
							{
								flag = 1;
								break;
							}
						}
						if (flag == 0)
						m_deleteSpacingRelations.push_back(m_SpacingRelations[i][0]);
					}
				}
			}
			else
				ite++;
		}
	}
	//std::cout << "delete=============================m_deleteSpacingRelations size is " << m_deleteSpacingRelations.size() << "\n";
	//std::cout << "delete=============================m_addSpacingRelations size is " << m_addSpacingRelations.size() << "\n";

}

bool CDisplayWidget::TwoSpacingRelaionIsEqual(RecordNo no1, RecordNo no2)
{
	if (no1.shape_no1 == no2.shape_no1 && no1.edge_no1 == no2.edge_no1&& no1.vertices_no1 == no2.vertices_no1)
	{
		if (no1.shape_no2 == no2.shape_no2 && no1.edge_no2 == no2.edge_no2&& no1.vertices_no2 == no2.vertices_no2)
		{
			return true;
		}
	}
	if (no1.shape_no1 == no2.shape_no2 && no1.edge_no1 == no2.edge_no2&& no1.vertices_no1 == no2.vertices_no2)
	{
		if (no1.shape_no2 == no2.shape_no1 && no1.edge_no2 == no2.edge_no1&& no1.vertices_no2 == no2.vertices_no1)
		{
			return true;
		}
	}
	return false;
}

void CDisplayWidget::AddSpacingRelations()
{

	CShapeHandler* handler;
	handler = m_shapeHandler_h;
	if (handler->m_allParts.empty())
		return;
	if (m_spacingStrokeAL.size() < 2)
	{
		return;
	}

	bool isAddRelaion = true;
	//编号
	handler->ComputeEdgeAndNormal(this->camera());//计算边和法线
	for (int i = 0; i < handler->m_allParts.size(); i++)
	{
		for (int j = 0; j < handler->m_allParts[i]->edges.size(); j++)
		{
			handler->m_allParts[i]->edges[j].m_shapeIndex = i;
		}
	}
	cv::Vec2d startPoint = m_spacingStrokeAL.front();
	cv::Vec2d endPoint = m_spacingStrokeAL.back();
	//起点
	Edge startEdge;
	double dis = 100;
	for (int i = 0; i < m_shapeHandler_h->m_allParts.size(); i++)
	{
		for (int j = 0; j < m_shapeHandler_h->m_allParts[i]->edges.size(); j++)
		{
			Edge e = m_shapeHandler_h->m_allParts[i]->edges[j];
			if (e.m_isVisible)
			{
				for (int k = 0; k < e.m_points.size(); k++)
				{
					if (dis > Utils::Calculate2DTwoPointDistance(startPoint, e.m_points[k]))
					{
						startEdge = e;
						dis = Utils::Calculate2DTwoPointDistance(startPoint, e.m_points[k]);
					}
				}

			}
		}
	}
	if (dis > 30)
		isAddRelaion = false;
	//终点
	Edge endEdge;
	dis = 100;
	for (int i = 0; i < m_shapeHandler_h->m_allParts.size(); i++)
	{
		for (int j = 0; j < m_shapeHandler_h->m_allParts[i]->edges.size(); j++)
		{
			Edge e = m_shapeHandler_h->m_allParts[i]->edges[j];
			//
			cv::Vec2d dir1, dir2;
			dir1 = endPoint - startPoint;
			dir2 = e.m_dir2D;
			if (e.m_isVisible && (abs(dir1.dot(dir2))>0.7 || isnan(abs(dir1.dot(dir2)))))//&& (abs(dir1.dot(dir2))>0.7 || isnan(abs(dir1.dot(dir2))))
			{
				for (int k = 0; k < e.m_points.size(); k++)
				{
					if (dis > Utils::Calculate2DTwoPointDistance(endPoint, e.m_points[k]))
					{
						endEdge = e;
						dis = Utils::Calculate2DTwoPointDistance(endPoint, e.m_points[k]);
					}
				}

			}
		}
	}

	if (dis > 30)
		isAddRelaion = false;
	if (startEdge.m_shapeIndex == endEdge.m_shapeIndex)
		isAddRelaion = false;
	if (abs(startEdge.m_dir3D.dot(endEdge.m_dir3D)) > 0.9 || isnan(abs(startEdge.m_dir3D.dot(endEdge.m_dir3D))))
	{
	}
	else
		isAddRelaion = false;
	//距离限制
	float distance = handler->ComputEdgeToLineDis(startEdge, endEdge);//计算两条边的距离
	if (distance > 50)
		isAddRelaion = false;
	if (isAddRelaion)
	{
		ShapeRelation shapeRelation;
		shapeRelation.edges[0] = startEdge;
		shapeRelation.edges[1] = endEdge;
		RecordNo recordNo;
		double dis1 = Utils::Calculate2DTwoPointDistance(startEdge.m_projectPt2D[0], endEdge.m_projectPt2D[0]);
		double dis2 = Utils::Calculate2DTwoPointDistance(startEdge.m_projectPt2D[0], endEdge.m_projectPt2D[1]);
		double dis3 = Utils::Calculate2DTwoPointDistance(startEdge.m_projectPt2D[1], endEdge.m_projectPt2D[0]);
		double dis4 = Utils::Calculate2DTwoPointDistance(startEdge.m_projectPt2D[1], endEdge.m_projectPt2D[1]);
		double min = dis1;
		cv::Vec2d point1, point2;
		if (dis2 < min)
			min = dis2;
		if (dis3 < min)
			min = dis3;
		if (dis4 < min)
			min = dis4;
		if (dis1 == min)
		{
			point1 = startEdge.m_projectPt2D[0];
			point2 = endEdge.m_projectPt2D[0];
		}
		if (dis2 == min)
		{
			point1 = startEdge.m_projectPt2D[0];
			point2 = endEdge.m_projectPt2D[1];
		}
		if (dis3 == min)
		{
			point1 = startEdge.m_projectPt2D[1];
			point2 = endEdge.m_projectPt2D[0];
		}
		if (dis4 == min)
		{
			point1 = startEdge.m_projectPt2D[1];
			point2 = endEdge.m_projectPt2D[1];
		}
		QPointF point, p1, p2;
		p1.setX(point1[0]);
		p1.setY(point1[1]);
		p2.setX(point2[0]);
		p2.setY(point2[1]);
		for (int s = 0; s < handler->m_allParts.size(); s++)
		{
			for (int v = 0; v < handler->m_allParts[s]->edges.size(); v++)
			{
				point.setX(handler->m_allParts[s]->edges[v].m_projectPt2D[0][0]);
				point.setY(handler->m_allParts[s]->edges[v].m_projectPt2D[0][1]);
				if (point == p1)
				{
					recordNo.edge_no1 = v;
					recordNo.shape_no1 = s;
					recordNo.vertices_no1 = 0;
					break;
				}
				point.setX(handler->m_allParts[s]->edges[v].m_projectPt2D[1][0]);
				point.setY(handler->m_allParts[s]->edges[v].m_projectPt2D[1][1]);
				if (point == p1)
				{
					recordNo.edge_no1 = v;
					recordNo.shape_no1 = s;
					recordNo.vertices_no1 = 1;
					break;
				}
			}
			for (int v = 0; v < handler->m_allParts[s]->edges.size(); v++)
			{
				point.setX(handler->m_allParts[s]->edges[v].m_projectPt2D[0][0]);
				point.setY(handler->m_allParts[s]->edges[v].m_projectPt2D[0][1]);
				if (point == p2)
				{
					recordNo.edge_no2 = v;
					recordNo.shape_no2 = s;
					recordNo.vertices_no2 = 0;
					break;
				}
				point.setX(handler->m_allParts[s]->edges[v].m_projectPt2D[1][0]);
				point.setY(handler->m_allParts[s]->edges[v].m_projectPt2D[1][1]);
				if (point == p2)
				{
					recordNo.edge_no2 = v;
					recordNo.shape_no2 = s;
					recordNo.vertices_no2 = 1;
					break;
				}
			}
		}
		shapeRelation.recordNo = recordNo;
		int flag = 0;
		for (int k = 0; k < m_addSpacingRelations.size(); k++)
		{
			if (TwoSpacingRelaionIsEqual(m_addSpacingRelations[k].recordNo, recordNo))
			{
				flag = 1;
				break;
			}
		}
		if (flag == 0)
			m_addSpacingRelations.push_back(shapeRelation);
		//如果delete里面有这个添加的关系，则删除
		for (vector<RecordNo>::iterator it = m_deleteSpacingRelations.begin(); it != m_deleteSpacingRelations.end();)
		{
			if (TwoSpacingRelaionIsEqual(*it, recordNo))
			{
				it = m_deleteSpacingRelations.erase(it);
				break;
			}
			else
			{
				it++;
			}
		}
	}
	//std::cout << "add=============================m_deleteSpacingRelations size is " << m_deleteSpacingRelations.size() << "\n";
	//std::cout << "add=============================m_addSpacingRelations size is " << m_addSpacingRelations.size() << "\n";
}

void CDisplayWidget::GetFinalSpacingRelaions(std::vector<std::vector<RecordNo>> group)
{
	//std::cout << "delete=============================m_deleteSpacingRelations size is " << m_deleteSpacingRelations.size() << "\n";
	//如果有delete里面的内容则则删除
	for (int k = 0; k < m_deleteSpacingRelations.size();k++)
	{
		RecordNo deleteNo = m_deleteSpacingRelations[k];
		for (int i = 0; i < m_SpacingRelations.size(); i++)
		{
			for (vector<RecordNo>::iterator it = m_SpacingRelations[i].begin(); it != m_SpacingRelations[i].end();it++)
			{
				if ((deleteNo.shape_no1 == it->shape_no1&&deleteNo.shape_no2 == it->shape_no2) || (deleteNo.shape_no2 == it->shape_no1&&deleteNo.shape_no1 == it->shape_no2))
				{
					m_SpacingRelations[i].erase(it);
					break;
				}
			}
		
		}
	}
  //如果有group里面的内容则删除
	for (int i = 0; i < group.size(); i++)
	{
		for (int j = 0; j < group[i].size(); j++)
		{
			RecordNo deleteNo = group[i][j];
			for (int k = 0; k < m_SpacingRelations.size(); k++)
			{
				for (vector<RecordNo>::iterator it = m_SpacingRelations[k].begin(); it != m_SpacingRelations[k].end(); it++)
				{
					if ((deleteNo.shape_no1 == it->shape_no1&&deleteNo.shape_no2 == it->shape_no2) || (deleteNo.shape_no2 == it->shape_no1&&deleteNo.shape_no1 == it->shape_no2))
					{
						m_SpacingRelations[k].erase(it);
						break;
					}
				}
			}
		}
	}
	//添加group
	for (int i = 0; i < group.size();i++)
   {
	   m_SpacingRelations.push_back(group[i]);
   }
	
  //如果m_SpacingRelations每组剩余一个则删除
	for (std::vector<std::vector<RecordNo>>::iterator it = m_SpacingRelations.begin(); it != m_SpacingRelations.end(); )
	{
		if (it->size()<2)
		{
			it=m_SpacingRelations.erase(it);
		}
		else
		{
			it++;
		}
	}

}
