#include "oacreater.h"

OACreater::OACreater(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	ManuallySetupUI();
	ChageMouseToOperateShape();
	TransferData();
	m_appName = "MyViewer";
	this->setWindowTitle(m_appName);
	UpdateUndoRedoButton();
	m_shapeId = 0;
	gdv_isFourRelation = false;
	gdv_isRotateWithCamera = false;
	ui.displaywidget->m_stopShapeRotation = true;
	//user study
	ui.actionEnter_User_Name->setEnabled(false);
	ui.actionStart_User_Study->setEnabled(false);
	ui.actionProceed_User_Study->setEnabled(false);
	ui.actionEnd_User_Study->setEnabled(false);
	ui.actionBegin_User_Study_One->setEnabled(false);
	ui.actionFinish_User_Study->setEnabled(false);
	ui.actionCurve_Alignment->setEnabled(false);
	ui.actionSelect_Shape->setEnabled(false);
	m_writerp = NULL;
}

OACreater::~OACreater()
{

}

void OACreater::OpenShape()
{
	ChageMouseToOperateShape();
	static const QString defaultPath("DEFAULT_SHAPE_PATH");
	QSettings thisSetting;
	QString path = thisSetting.value(defaultPath).toString();
	QString filter = "(*.obj *.off *.model);;(*.*)";
	m_filePath = QFileDialog::getOpenFileName(this, tr("Open Shape"), path, filter);
	if (!m_filePath.isEmpty())
	{
		std::string name = m_filePath.toStdString();
		std::string ext = m_filePath.toStdString();
		size_t found = ext.find_last_of('.');
		ext = ext.substr(found + 1);
		if (ext == "obj" || ext == "off")
		{
			AddNewPart(name.c_str());
		}
		else if (ext == "model")
		{
			LoadWholeModel(name.c_str());
		}
		QDir crtDir;
		found = name.find_last_of('/');
		name = name.substr(0, found);
		QString nameQ = QString::fromStdString(name);
		thisSetting.setValue(defaultPath, crtDir.absoluteFilePath(nameQ));
	}
	ui.displaywidget->m_stopRelationUpdate = false;
	ui.displaywidget->m_recordRelations.clear();
	ui.displaywidget->m_optRelations.clear();
	ui.displaywidget->m_SpacingRelations.clear();
	ui.displaywidget->repaint();
	ui.displaywidget->UpdateRelations();
	ui.displaywidget->setManipulatedFrame(ui.displaywidget->camera()->frame());
	ui.displaywidget->m_stopShapeRotation = true;
	ui.displaywidget->repaint();
}

void OACreater::SaveShape()
{
	ChageMouseToOperateShape();
	static const QString defaultPath("DEFAULT_SHAPE_PATH");
	QSettings thisSetting;
	QString path = thisSetting.value(defaultPath).toString();
	QString filter = "(*.model);;(*.*)";
	QString filePath = QFileDialog::getSaveFileName(this, tr("Save Shape"), path, filter);
	if (!filePath.isEmpty())
	{
		std::string name = filePath.toStdString();
		QDir crtDir;
		std::vector<bool> floatingFlag;
		for (int i = 0; i < m_shapeHandler_p.m_allParts.size();i++)
		{
			floatingFlag.push_back(m_shapeHandler_p.m_allParts[i]->m_isFloating);
		}
		FixAllModels();
		m_shapeHandler_p.SaveWholeModel(name.c_str(),floatingFlag);

		std::string dataDir = name;
		size_t found = dataDir.find_last_of('.');
		dataDir = dataDir.substr(0, found);
		dataDir = dataDir + "_data";
		std::string thisViewPath = dataDir + "/" + "camera.vmat";
		CViewMat thisViewMat;
		thisViewMat = *(ui.displaywidget->camera()->frame());
		thisViewMat.SaveView(thisViewPath.c_str());
		found = name.find_last_of('/');
		name = name.substr(0, found);
		QString nameQ = QString::fromStdString(name);
		thisSetting.setValue(defaultPath, crtDir.absoluteFilePath(nameQ));
		SetFloating(floatingFlag);
	}
	ui.displaywidget->repaint();
	ui.displaywidget->UpdateRelations();
	ui.displaywidget->setManipulatedFrame(ui.displaywidget->camera()->frame());
	//FloatAllModels();
}

void OACreater::Undo()
{
	//ChageMouseToOperateShape();
	ui.displaywidget->setManipulatedFrame(ui.displaywidget->camera()->frame());
	if (m_listShapeHandler_history.size() > 0)
	{
		m_listShapeHandler_frontHistory.push_back(m_shapeHandler_p);
		m_listCamera_frontHistory.push_back(*(ui.displaywidget->camera()->frame()));
		m_shapeHandler_p = m_listShapeHandler_history.back();
		m_frame_backup = m_listCamera_history.back();
		m_listShapeHandler_history.pop_back();
		m_listCamera_history.pop_back();
		ui.displaywidget->SetData(&m_shapeHandler_p);
		//editRelaions
		EditRelaion editRelaion;
		editRelaion.m_addRelations = ui.displaywidget->m_addRelations;
		editRelaion.m_deleteRelations = ui.displaywidget->m_deleteRelations;
		editRelaion.m_optRelations = ui.displaywidget->m_optRelations;
		editRelaion.m_recordRelations = ui.displaywidget->m_recordRelations;
		m_editRelaion_frontHistory.push_back(editRelaion);
		editRelaion = m_editRelaion_history.back();
		m_editRelaion_history.pop_back();
		ui.displaywidget->m_addRelations = editRelaion.m_addRelations;
		ui.displaywidget->m_deleteRelations = editRelaion.m_deleteRelations;
		ui.displaywidget->m_optRelations = editRelaion.m_optRelations;
		ui.displaywidget->m_recordRelations = editRelaion.m_recordRelations;
		//spacingRelaiong
		SpacingRelaion spacingRelation;
		spacingRelation.m_addSpacingRelations = ui.displaywidget->m_addSpacingRelations;
		spacingRelation.m_addSpacingRelationsGroup = ui.displaywidget->m_addSpacingRelationsGroup;
		spacingRelation.m_deleteSpacingRelations = ui.displaywidget->m_deleteSpacingRelations;
		spacingRelation.m_SpacingRelations = ui.displaywidget->m_SpacingRelations;
		m_spacingRelation_frontHistory.push_back(spacingRelation);
		spacingRelation = m_spacingRelation_history.back();
		m_spacingRelation_history.pop_back();
		ui.displaywidget->m_addSpacingRelations = spacingRelation.m_addSpacingRelations;
		ui.displaywidget->m_addSpacingRelationsGroup = spacingRelation.m_addSpacingRelationsGroup;
		ui.displaywidget->m_deleteSpacingRelations = spacingRelation.m_deleteSpacingRelations;
		ui.displaywidget->m_SpacingRelations = spacingRelation.m_SpacingRelations;
		//
		ui.displaywidget->camera()->interpolateTo(m_frame_backup, 0.001);
	}
	UpdateUndoRedoButton();
	if (m_writerp != NULL)
	{
		(*m_writerp) << "UNDO " << (double)clock() << std::endl;
		m_writerp->flush();
	}
	ui.displaywidget->UpdateRelations();
}

void OACreater::Redo()
{
	//ChageMouseToOperateShape();
	ui.displaywidget->setManipulatedFrame(ui.displaywidget->camera()->frame());
	if (m_listShapeHandler_frontHistory.size() > 0)
	{
		m_listShapeHandler_history.push_back(m_shapeHandler_p);
		m_listCamera_history.push_back(*(ui.displaywidget->camera()->frame()));
		m_shapeHandler_p = m_listShapeHandler_frontHistory.back();
		m_frame_backup = m_listCamera_frontHistory.back();
		m_listShapeHandler_frontHistory.pop_back();
		m_listCamera_frontHistory.pop_back();
		ui.displaywidget->SetData(&m_shapeHandler_p);
		//editRelaions
		EditRelaion editRelaion;
		editRelaion.m_addRelations = ui.displaywidget->m_addRelations;
		editRelaion.m_deleteRelations = ui.displaywidget->m_deleteRelations;
		editRelaion.m_optRelations = ui.displaywidget->m_optRelations;
		editRelaion.m_recordRelations = ui.displaywidget->m_recordRelations;
		m_editRelaion_history.push_back(editRelaion);
		editRelaion = m_editRelaion_frontHistory.back();
		m_editRelaion_frontHistory.pop_back();
		ui.displaywidget->m_addRelations = editRelaion.m_addRelations;
		ui.displaywidget->m_deleteRelations = editRelaion.m_deleteRelations;
		ui.displaywidget->m_optRelations = editRelaion.m_optRelations;
		ui.displaywidget->m_recordRelations = editRelaion.m_recordRelations;
		//spacingRelations
		SpacingRelaion spacingRelation;
		spacingRelation.m_addSpacingRelations = ui.displaywidget->m_addSpacingRelations;
		spacingRelation.m_addSpacingRelationsGroup = ui.displaywidget->m_addSpacingRelationsGroup;
		spacingRelation.m_deleteSpacingRelations = ui.displaywidget->m_deleteSpacingRelations;
		spacingRelation.m_SpacingRelations = ui.displaywidget->m_SpacingRelations;
		m_spacingRelation_history.push_back(spacingRelation);
		spacingRelation = m_spacingRelation_frontHistory.back();
		m_spacingRelation_frontHistory.pop_back();
		ui.displaywidget->m_addSpacingRelations = spacingRelation.m_addSpacingRelations;
		ui.displaywidget->m_addSpacingRelationsGroup = spacingRelation.m_addSpacingRelationsGroup;
		ui.displaywidget->m_deleteSpacingRelations = spacingRelation.m_deleteSpacingRelations;
		ui.displaywidget->m_SpacingRelations = spacingRelation.m_SpacingRelations;
		//
		ui.displaywidget->camera()->interpolateTo(m_frame_backup, 0.001);
	}
	UpdateUndoRedoButton();
	if (m_writerp != NULL)
	{
		(*m_writerp) << "REDO " << (double)clock() << std::endl;
		m_writerp->flush();
	}
	ui.displaywidget->UpdateRelations();
}

void OACreater::Copy()
{
	
	
	if (ui.displaywidget->m_shapePart==NULL)
	{
		m_copiedShape.m_mesh = NULL;
		return;
	}
	m_pasteCount = 1;
	for (int i = 0; i < m_shapeHandler_p.m_allParts.size(); i++)
	{
		if (m_shapeHandler_p.m_allParts[i]->m_frame == ui.displaywidget->manipulatedFrame())
		{
			m_copiedShape = *m_shapeHandler_p.m_allParts[i];
			break;
		}
	}
	if (m_writerp != NULL)
	{
		(*m_writerp) << "COPY " << (double)clock() << std::endl;
		m_writerp->flush();
	}
	ChageMouseToOperateShape();
}

void OACreater::Paste()
{
	if (m_copiedShape.m_mesh == NULL)
		return;
	
	CShapePart shapePart = m_copiedShape;
	shapePart.m_frame->translate(0.05*m_pasteCount, 0.05*m_pasteCount, 0.05*m_pasteCount);
	m_pasteCount++;
	shapePart.m_id = m_shapeId;
	m_shapeId++;
	m_shapeHandler_p.AddNewPart(shapePart);
	ui.displaywidget->UpdateManipulatedFrame();
	ui.displaywidget->repaint();
	if (m_writerp != NULL)
	{
		(*m_writerp) << "PASTE " << (double)clock() << std::endl;
		m_writerp->flush();
	}
	ChageMouseToOperateShape();
	//清除记录的关系
	//ClearEditRelaion();
	//BackupDataReceiver();
}

void OACreater::Delete()
{
	if (m_shapeHandler_p.m_allParts.size() == 0)
		return;
	
	//StopRelationUpdate();
	ui.displaywidget->m_addRelations.clear();
	ui.displaywidget->m_deleteRelations.clear();
	ui.displaywidget->m_recordRelations.clear();
	ui.displaywidget->m_optRelations.clear();
	for (int i = 0; i < m_shapeHandler_p.m_allParts.size();i++)
    {
		if (m_shapeHandler_p.m_allParts[i]->m_frame == ui.displaywidget->manipulatedFrame())
		{
			m_shapeHandler_p.m_allParts.erase(m_shapeHandler_p.m_allParts.begin() + i);
			break;
		}
    }
	if (m_writerp != NULL)
	{
		(*m_writerp) << "DELETE_SHAPE " << (double)clock() << std::endl;
		m_writerp->flush();
	}
	ui.displaywidget->setManipulatedFrame(ui.displaywidget->camera()->frame());
	ChageMouseToOperateShape();
	ui.displaywidget->UpdateRelations();
	ui.displaywidget->repaint();
}

void OACreater::SelectModel()
{
	ChageMouseToOperateShape();
	for(int i = 0; i < m_shapeHandler_p.m_allParts.size(); i++)
	{
		if (m_shapeHandler_p.m_allParts[i]->m_frame == ui.displaywidget->manipulatedFrame())
		{
			if (m_shapeHandler_p.m_allParts[i]->m_isFloating)
			{
				m_shapeHandler_p.m_allParts[i]->m_isFloating = false;
				//切换float/fix的时候保持位置不变
				GLdouble matrix[16];
				for (int k = 0; k < 16; k++)
				{
					matrix[k] = m_shapeHandler_p.m_allParts[i]->m_frame->matrix()[k];
				}
				GLdouble camera_matrix[16];
				for (int k = 0; k < 16; k++)
				{
					camera_matrix[k] = ui.displaywidget->camera()->frame()->matrix()[k];
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
				m_shapeHandler_p.m_allParts[i]->m_frame->setFromMatrix(matrix);
				// 设置参考frame为NULL，解除悬浮状态，使其具有位置
				m_shapeHandler_p.m_allParts[i]->m_frame->setReferenceFrame(NULL);
			}
			else
			{
				m_shapeHandler_p.m_allParts[i]->m_isFloating = true;
				//切换float/fix的时候保持位置不变
				GLdouble camera_matrix[16];
				cv::Mat mat_m = cv::Mat::zeros(4, 4, CV_64F);
				for (int m = 0; m < 4; m++)
				{
					for (int n = 0; n < 4; n++)
					{
						mat_m.at<double>(m, n) = ui.displaywidget->camera()->frame()->matrix()[m * 4 + n];
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
					matrix[k] = m_shapeHandler_p.m_allParts[i]->m_frame->matrix()[k];
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
				m_shapeHandler_p.m_allParts[i]->m_frame->setFromMatrix(matrix);
				//设置参考frame为相机，使位置处于悬浮状态
				m_shapeHandler_p.m_allParts[i]->m_frame->setReferenceFrame(ui.displaywidget->camera()->frame());
			}
		}
	}
	ui.displaywidget->UpdateRelations();
	ui.displaywidget->repaint();
}

void OACreater::FixAllModels()
{
	ChageMouseToOperateShape();
	for (int i = 0; i < m_shapeHandler_p.m_allParts.size(); i++)
	{
		if (m_shapeHandler_p.m_allParts[i]->m_isFloating)
		{
			m_shapeHandler_p.m_allParts[i]->m_isFloating = false;
			//切换float/fix的时候保持位置不变
			GLdouble matrix[16];
			for (int k = 0; k < 16; k++)
			{
				matrix[k] = m_shapeHandler_p.m_allParts[i]->m_frame->matrix()[k];
			}
			GLdouble camera_matrix[16];
			for (int k = 0; k < 16; k++)
			{
				camera_matrix[k] = ui.displaywidget->camera()->frame()->matrix()[k];
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
			m_shapeHandler_p.m_allParts[i]->m_frame->setFromMatrix(matrix);
			// 设置参考frame为NULL，解除悬浮状态，使其具有位置
			m_shapeHandler_p.m_allParts[i]->m_frame->setReferenceFrame(NULL);
		}
	}
	ui.displaywidget->UpdateRelations();
	ui.displaywidget->repaint();
}

void OACreater::FloatAllModels()
{
	ChageMouseToOperateShape();
	for (int i = 0; i < m_shapeHandler_p.m_allParts.size(); i++)
	{
		if (!m_shapeHandler_p.m_allParts[i]->m_isFloating)
		{
			m_shapeHandler_p.m_allParts[i]->m_isFloating = true;
			//切换float/fix的时候保持位置不变
			GLdouble camera_matrix[16];
			cv::Mat mat_m = cv::Mat::zeros(4, 4, CV_64F);
			for (int m = 0; m < 4; m++)
			{
				for (int n = 0; n < 4; n++)
				{
					mat_m.at<double>(m, n) = ui.displaywidget->camera()->frame()->matrix()[m * 4 + n];
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
				matrix[k] = m_shapeHandler_p.m_allParts[i]->m_frame->matrix()[k];
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
			m_shapeHandler_p.m_allParts[i]->m_frame->setFromMatrix(matrix);
			//设置参考frame为相机，使位置处于悬浮状态
			m_shapeHandler_p.m_allParts[i]->m_frame->setReferenceFrame(ui.displaywidget->camera()->frame());
		}
	}
	ui.displaywidget->UpdateRelations();
	ui.displaywidget->repaint();
}

void OACreater::UpdateRelations(bool thisFlag)
{
	ChageMouseToOperateShape();
	if (thisFlag)
	{
		ui.displaywidget->m_stopRelationUpdate = false;
		ui.displaywidget->UpdateRelations();
	}
	else
	{
		ui.displaywidget->m_stopRelationUpdate = true;
		ClearFrontDate();
	}
	ui.displaywidget->repaint();

}

void OACreater::StopRelationUpdate()
{
	ChageMouseToOperateShape();
	ui.displaywidget->m_stopRelationUpdate = true;
	ui.displaywidget->m_addRelations.clear();
	ui.displaywidget->m_deleteRelations.clear();
	ui.displaywidget->m_recordRelations.clear();
	ui.displaywidget->m_optRelations.clear();
	ui.displaywidget->repaint();
	SetPreviewDate();

}

void OACreater::ApplyRefinement()
{
	//ChageMouseToOperateShape();
	if (m_writerp != NULL)
	{
		(*m_writerp) << "REFINEMENT_BEGIN " << (double)clock() << std::endl;
		m_writerp->flush();
	}
	ui.displaywidget->ExecuteAlignmentOptimizing();
	if (m_writerp != NULL)
	{
		(*m_writerp) << "REFINEMENT_END " << (double)clock() << std::endl;
		m_writerp->flush();
	}
	if (ui.displaywidget->m_mouseOperateType == SC::DeleteRelations || ui.displaywidget->m_mouseOperateType == SC::AddRelations)
	{
		ChageMouseToOperateShape();
	}
	ui.displaywidget->setManipulatedFrame(ui.displaywidget->camera()->frame());
	
}

void OACreater::AddRelaions()
{
	DisableAllOperationState();
	ui.actionAdd_relations->setChecked(true);
	ui.displaywidget->clearMouseBindings();
	if (ui.displaywidget->m_mouseOperateType == SC::RecSelect)
	{
		ui.displaywidget->m_optRelations.clear();
	}
	ui.displaywidget->m_mouseOperateType = SC::AddRelations;
	ui.displaywidget->setManipulatedFrame(ui.displaywidget->camera()->frame());
	//ui.displaywidget->EditRelations();
	ui.displaywidget->UpdateRelations();
	ui.displaywidget->repaint();
}
void OACreater::ClearEditRelaions()
{
	ui.displaywidget->m_addRelations.clear();
	ui.displaywidget->m_deleteRelations.clear();
	ui.displaywidget->UpdateRelations();
	ui.displaywidget->repaint();
}
void OACreater::SelectShape()
{
	DisableAllOperationState();
	ui.actionSelect_Shape->setChecked(true);
	ui.displaywidget->clearMouseBindings();
	ui.displaywidget->m_mouseOperateType = SC::RecSelect;
	ui.displaywidget->setManipulatedFrame(ui.displaywidget->camera()->frame());
	ClearEditRelaions();
	ui.displaywidget->repaint();
}

void OACreater::ChageMouseToOperateShape()
{
	DisableAllOperationState();
	ui.displaywidget->m_isClearMouseBinding = false;
	ui.actionOperate_Shape->setChecked(true);
	ui.displaywidget->InitializeMouseBinding();
	ui.displaywidget->m_mouseOperateType = SC::OperateShape;
	//ui.displaywidget->setManipulatedFrame(ui.displaywidget->camera()->frame());
	ui.displaywidget->repaint();
}

void OACreater::CurveAlignment()
{
	DisableAllOperationState();
	ui.actionCurve_Alignment->setChecked(true);
	ui.displaywidget->clearMouseBindings();
	ui.displaywidget->m_mouseOperateType = SC::CurveAlignment;
	ui.displaywidget->setManipulatedFrame(ui.displaywidget->camera()->frame());
	ui.displaywidget->repaint();
}

void OACreater::ChangeScale()
{
	DisableAllOperationState();
	ui.actionScale->setChecked(true);
	ui.displaywidget->m_mouseOperateType = SC::ChangeScale;
	ui.displaywidget->repaint();
}
void OACreater::DisableAllOperationState()
{
	ui.actionCurve_Alignment->setChecked(false);
	ui.actionAdd_relations->setChecked(false);
	ui.actionDelete_Relations->setChecked(false);
	ui.actionSelect_Shape->setChecked(false);
	ui.actionOperate_Shape->setChecked(false);
	ui.actionScale->setChecked(false);
	ui.actionOperate_Camera->setChecked(false);
	//spacing
	ui.actionAdd_Spacing_Start->setChecked(false);
	ui.actionRemove_Spacing->setChecked(false);
	ui.displaywidget->m_curveStrokeAL.clear();
	ui.displaywidget->m_relationStrokeAL.clear();
	ui.displaywidget->m_spacingStrokeAL.clear();
	ui.displaywidget->m_partShapeHandler_h.DestoryAllParts();
	ui.displaywidget->m_isClearMouseBinding = true;
}

void OACreater::AddNewPart(const char* shapePartPath)
{
	CShapePart* newShapePart = new CShapePart();
	newShapePart->LoadPart(shapePartPath);
	//
	newShapePart->m_isFloating = true;
	//切换float/fix的时候保持位置不变
	GLdouble camera_matrix[16];
	cv::Mat mat_m = cv::Mat::zeros(4, 4, CV_64F);
	for (int m = 0; m < 4; m++)
	{
		for (int n = 0; n < 4; n++)
		{
			mat_m.at<double>(m, n) = ui.displaywidget->camera()->frame()->matrix()[m * 4 + n];
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
		matrix[k] = newShapePart->m_frame->matrix()[k];
	}
	Utils::MatrixMult(matrix, camera_matrix);
	//glMultMatrixd(camera_matrix);
	newShapePart->m_frame->setFromMatrix(matrix);
	//设置参考frame为相机，使位置处于悬浮状态
	newShapePart->m_frame->setReferenceFrame(ui.displaywidget->camera()->frame());
    newShapePart->NormalizeMesh();
	newShapePart->m_id = m_shapeId;
	m_shapeId++;
	m_shapeHandler_p.AddNewPart(newShapePart);
	ui.displaywidget->UpdateManipulatedFrame();
}

void OACreater::LoadWholeModel(const char* modelPath)
{
	m_shapeId = 0;
	m_shapeHandler_p.DestoryAllParts();
	std::fstream fin;
	fin.open(modelPath, std::ios::in);
	int numOfParts;
	fin >> numOfParts;
	std::vector<bool> floating;
	for (int i = 0; i < numOfParts;i++)
	{
		bool temp;
		fin >> temp;
		floating.push_back(temp);
	}
	//
	QString fileName = QString(QLatin1String(modelPath));
	int index;
	index = fileName.lastIndexOf(".");
	fileName.truncate(index);//把它cate掉
	std::string fname = fileName.toStdString();
	for (int i = 0; i < numOfParts; ++i)
	{
		CShapePart* newShapePart = new CShapePart();
		std::string shapePartPath, shapeViewPath;
		fin >> shapePartPath >> shapeViewPath;
		shapePartPath = fname + shapePartPath;
		newShapePart->LoadPart(shapePartPath.c_str());
		CViewMat thisViewMat;
		shapeViewPath = fname + shapeViewPath;
		thisViewMat.LoadView(shapeViewPath.c_str());
		double m[16];
		thisViewMat.GetViewMat(m);
		newShapePart->m_frame->setFromMatrix(m);
		//newShapePart->InitializeLineDrawing();
		newShapePart->m_id = m_shapeId;
		m_shapeId++;
		m_shapeHandler_p.AddNewPart(newShapePart);
	}
	std::string cameraViewPath;
	fin >> cameraViewPath;
	CViewMat thisViewMat;
	cameraViewPath = fname + cameraViewPath;
	thisViewMat.LoadView(cameraViewPath.c_str());
	double m[16];
	thisViewMat.GetViewMat(m);
	ui.displaywidget->camera()->frame()->setFromMatrix(m);
	SetFloating(floating);
	ui.displaywidget->UpdateManipulatedFrame();
	//清除记录的关系
	ClearFrontDate();
	ChageMouseToOperateShape();
	ui.displaywidget->repaint();
	
}

void OACreater::TransferData()
{
	ui.displaywidget->SetData(&m_shapeHandler_p);
	ui.displaywidget->UpdateManipulatedFrame();
}

void OACreater::UpdateUndoRedoButton()
{
	if (m_listShapeHandler_history.size() > 0)
	{
		ui.actionUndo->setEnabled(true);
	}
	else
	{
		ui.actionUndo->setEnabled(false);
	}

	if (m_listShapeHandler_frontHistory.size() > 0)
	{
		ui.actionRedo->setEnabled(true);
	}
	else
	{
		ui.actionRedo->setEnabled(false);
	}
}

void OACreater::BackupDataReceiver()
{
	qglviewer::Frame lv_frame;
	CShapeHandler lv_handler;
	EditRelaion editRelation;
	SpacingRelaion spacingRelation;
	m_frame_backup = *(ui.displaywidget->camera()->frame());
	if (m_listShapeHandler_history.size() > 0 && m_listCamera_history.size() > 0)
	{
		lv_handler = m_listShapeHandler_history.back();
		lv_frame = m_listCamera_history.back();
		editRelation = m_editRelaion_history.back();
		spacingRelation = m_spacingRelation_history.back();
		EditRelaion currentRelation;
		currentRelation.m_addRelations = ui.displaywidget->m_addRelations;
		currentRelation.m_deleteRelations = ui.displaywidget->m_deleteRelations;
		currentRelation.m_optRelations = ui.displaywidget->m_optRelations;
		currentRelation.m_recordRelations = ui.displaywidget->m_recordRelations;
		//
		SpacingRelaion currentSpacingRelation;
		currentSpacingRelation.m_addSpacingRelations = ui.displaywidget->m_addSpacingRelations;
		currentSpacingRelation.m_addSpacingRelationsGroup = ui.displaywidget->m_addSpacingRelationsGroup;
		currentSpacingRelation.m_deleteSpacingRelations = ui.displaywidget->m_deleteSpacingRelations;
		currentSpacingRelation.m_SpacingRelations = ui.displaywidget->m_SpacingRelations;
		if (currentSpacingRelation.m_addSpacingRelations.size() == spacingRelation.m_addSpacingRelations.size() && currentSpacingRelation.m_addSpacingRelationsGroup.size() == spacingRelation.m_addSpacingRelationsGroup.size() && currentSpacingRelation.m_deleteSpacingRelations.size() == spacingRelation.m_deleteSpacingRelations.size())
		if (currentRelation.m_addRelations.size() == editRelation.m_addRelations.size() && currentRelation.m_deleteRelations.size() == editRelation.m_deleteRelations.size())
		if (CompareScene(lv_handler, m_shapeHandler_p, lv_frame, m_frame_backup))
			return;
	}
	m_listShapeHandler_history.push_back(m_shapeHandler_p);
	m_listCamera_history.push_back(m_frame_backup);
	//editRelaions
	EditRelaion editRelaion;
	editRelaion.m_addRelations = ui.displaywidget->m_addRelations;
	editRelaion.m_deleteRelations = ui.displaywidget->m_deleteRelations;
	editRelaion.m_optRelations = ui.displaywidget->m_optRelations;
	editRelaion.m_recordRelations = ui.displaywidget->m_recordRelations;
	m_editRelaion_history.push_back(editRelaion);
	//spacingRelation
	SpacingRelaion sRelation;
	sRelation.m_addSpacingRelations = ui.displaywidget->m_addSpacingRelations;
	sRelation.m_addSpacingRelationsGroup = ui.displaywidget->m_addSpacingRelationsGroup;
	sRelation.m_deleteSpacingRelations = ui.displaywidget->m_deleteSpacingRelations;
	sRelation.m_SpacingRelations = ui.displaywidget->m_SpacingRelations;
	m_spacingRelation_history.push_back(sRelation);
	//
	m_listShapeHandler_frontHistory.clear();
	m_listCamera_frontHistory.clear();
	m_editRelaion_frontHistory.clear();
	m_spacingRelation_frontHistory.clear();
	if (m_listShapeHandler_history.size() > 40)
	{
		m_listShapeHandler_history.pop_front();
		m_listCamera_history.pop_front();
		m_editRelaion_history.pop_front();
		m_spacingRelation_history.pop_front();
	}
	UpdateUndoRedoButton();
}

bool OACreater::CompareScene(CShapeHandler shapeHandler1, CShapeHandler shapeHandler2, qglviewer::Frame frame1, qglviewer::Frame frame2)
{
	double ldv_frameMatrix1[16];
	double ldv_frameMatrix2[16];
	frame1.getMatrix(ldv_frameMatrix1);
	frame2.getMatrix(ldv_frameMatrix2);
	for (int i = 0; i < 16; i++)
	{
		if (ldv_frameMatrix1[i] != ldv_frameMatrix2[i])
			return false;
	}
	if (shapeHandler1.m_allParts.size() != shapeHandler2.m_allParts.size())
		return false;
	CShapePart* shapePart1;
	CShapePart* shapePart2;
	for (int i = 0; i < shapeHandler1.m_allParts.size(); i++)
	{
		double ldv_matrix1[16];
		double ldv_matrix2[16];
		shapePart1 = shapeHandler1.m_allParts[i];
		shapePart2 = shapeHandler2.m_allParts[i];
		shapePart1->m_frame->getMatrix(ldv_matrix1);
		shapePart2->m_frame->getMatrix(ldv_matrix2);
		for (int j = 0; j < 16; j++)
		{
			if (ldv_matrix1[j] != ldv_matrix2[j])
				return false;
		}
	}
	return true;
}

void OACreater::ManuallySetupUI()
{
	connect(ui.displaywidget, SIGNAL(BackupDataSender()), this, SLOT(BackupDataReceiver()));
	connect(ui.displaywidget, SIGNAL(FixAllModelsSender()), this, SLOT(FixAllModels()));
	connect(ui.displaywidget, SIGNAL(UpdateRelationsSender()), this, SLOT(SetPreviewDate()));
}

void OACreater::RemoveAndUpdateRelation()
{
	ui.displaywidget->m_recordRelations.clear();
	ui.displaywidget->UpdateRelations();
}

void OACreater::DrawShapeNo(bool thisFlag)
{
	if (thisFlag)
	{
		ui.displaywidget->m_isShowNo = true;
		ui.preview->m_isShowNo = true;
	}
	else
	{
		ui.displaywidget->m_isShowNo = false;
		ui.preview->m_isShowNo = false;
	}
	ui.displaywidget->repaint();
	ui.preview->repaint();
}

void OACreater::SetFloating(std::vector<bool> floatings)
{
	if (floatings.size() != m_shapeHandler_p.m_allParts.size())
		return;
	for (int i = 0; i < floatings.size();i++)
	{
		if (floatings[i])
		{
			if (!m_shapeHandler_p.m_allParts[i]->m_isFloating)
			{
				m_shapeHandler_p.m_allParts[i]->m_isFloating = true;
				//切换float/fix的时候保持位置不变
				GLdouble camera_matrix[16];
				cv::Mat mat_m = cv::Mat::zeros(4, 4, CV_64F);
				for (int m = 0; m < 4; m++)
				{
					for (int n = 0; n < 4; n++)
					{
						mat_m.at<double>(m, n) = ui.displaywidget->camera()->frame()->matrix()[m * 4 + n];
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
					matrix[k] = m_shapeHandler_p.m_allParts[i]->m_frame->matrix()[k];
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
				m_shapeHandler_p.m_allParts[i]->m_frame->setFromMatrix(matrix);
				//设置参考frame为相机，使位置处于悬浮状态
				m_shapeHandler_p.m_allParts[i]->m_frame->setReferenceFrame(ui.displaywidget->camera()->frame());
			}
		}
		else
		{
			if (m_shapeHandler_p.m_allParts[i]->m_isFloating)
			{
				
				m_shapeHandler_p.m_allParts[i]->m_isFloating = false;
				//切换float/fix的时候保持位置不变
				GLdouble matrix[16];
				for (int k = 0; k < 16; k++)
				{
					matrix[k] = m_shapeHandler_p.m_allParts[i]->m_frame->matrix()[k];
				}
				GLdouble camera_matrix[16];
				for (int k = 0; k < 16; k++)
				{
					camera_matrix[k] = ui.displaywidget->camera()->frame()->matrix()[k];
				}
				Utils::MatrixMult(matrix, camera_matrix);
				//glMultMatrixd(camera_matrix);
				//加入相机旋转矩阵
				camera_matrix[12] = 0;
				camera_matrix[13] = 0;
				camera_matrix[14] = 0;
				Utils::MatrixMult44(matrix, camera_matrix);
				//
				m_shapeHandler_p.m_allParts[i]->m_frame->setFromMatrix(matrix);
				// 设置参考frame为NULL，解除悬浮状态，使其具有位置
				m_shapeHandler_p.m_allParts[i]->m_frame->setReferenceFrame(NULL);
				
			}
		}
	}
}

void OACreater::SetPreviewDate()
{
	CShapeHandler handler=ui.displaywidget->GetPreviewResult();
	ui.preview->SetData(handler);
}

void OACreater::ClearEditRelaion()
{
	ui.displaywidget->m_addRelations.clear();
	ui.displaywidget->m_deleteRelations.clear();
}

void OACreater::StopRotation(bool thisFlag)
{
	if (!thisFlag)
	{
		ui.displaywidget->m_stopShapeRotation=true;
	}
	else
	{
		ui.displaywidget->m_stopShapeRotation=false;
	}
	ui.displaywidget->repaint();
}

void OACreater::ShowFourRelaions(bool thisFlag)
{
	if (thisFlag)
	{
		gdv_isFourRelation = true;
	}
	else
	{
		gdv_isFourRelation = false;
	}
	ui.displaywidget->UpdateRelations();
	ui.displaywidget->repaint();
}

void OACreater::RotateWithCamera(bool thisFlag)
{
	if (!thisFlag)
	{
		gdv_isRotateWithCamera = false;
		for (int i = 0; i < m_shapeHandler_p.m_allParts.size(); i++)
		{
			if (m_shapeHandler_p.m_allParts[i]->m_isFloating)
			{
				//切换float/fix的时候保持位置不变
				GLdouble matrix[16];
				for (int k = 0; k < 16; k++)
				{
					matrix[k] = m_shapeHandler_p.m_allParts[i]->m_frame->matrix()[k];
				}
				GLdouble camera_matrix[16];
				for (int k = 0; k < 16; k++)
				{
					camera_matrix[k] = ui.displaywidget->camera()->frame()->matrix()[k];
				}
				//加入相机旋转矩阵
				camera_matrix[12] = 0;
				camera_matrix[13] = 0;
				camera_matrix[14] = 0;
				Utils::MatrixMult44(matrix, camera_matrix);
				m_shapeHandler_p.m_allParts[i]->m_frame->setFromMatrix(matrix);
			}
		}
	}
	else
	{
		gdv_isRotateWithCamera = true;
		for (int i = 0; i < m_shapeHandler_p.m_allParts.size(); i++)
		{
			if (m_shapeHandler_p.m_allParts[i]->m_isFloating)
			{

				//切换float/fix的时候保持位置不变
				GLdouble matrix[16];
				for (int k = 0; k < 16; k++)
				{
					matrix[k] = m_shapeHandler_p.m_allParts[i]->m_frame->matrix()[k];
				}
				GLdouble camera_matrix[16];
				for (int k = 0; k < 16; k++)
				{
					camera_matrix[k] = ui.displaywidget->camera()->frame()->matrix()[k];
				}
				//加入相机旋转矩阵
				camera_matrix[12] = 0;
				camera_matrix[13] = 0;
				camera_matrix[14] = 0;

				//
				cv::Mat mat_m = cv::Mat::zeros(4, 4, CV_64F);
				for (int m = 0; m < 4; m++)
				{
					for (int n = 0; n < 4; n++)
					{
						mat_m.at<double>(m, n) = camera_matrix[m * 4 + n];
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
				//
				Utils::MatrixMult44(matrix, camera_matrix);
				m_shapeHandler_p.m_allParts[i]->m_frame->setFromMatrix(matrix);
				// 设置参考frame为NULL，解除悬浮状态，使其具有位置
				//m_shapeHandler_p.m_allParts[i]->m_frame->setReferenceFrame(NULL);
			}
		}
	}

	//std::cout <<gdv_isRotateWithCamera<<"\n";
	ui.displaywidget->UpdateRelations();
	ui.displaywidget->repaint();
}

void OACreater::LoadUserStudyPresets()
{
	m_useStudyPresets.clear();
	QSettings thisSetting;
	static const QString defaultPath("DEFAULT_PRESETS_PATH");
	QString path = thisSetting.value(defaultPath).toString();
	QString filter = "(*.usp);;(*.*)";
	 m_presetPath = QFileDialog::getOpenFileName(this, tr("Open File"), path, filter);
	if (!m_presetPath.isEmpty())
	{
		std::string name = m_presetPath.toStdString();
		QDir crtDir;
		LoadUSP2File(&name[0]);
		size_t found = name.find_last_of('/');
		std::cout << name << std::endl;
		name = name.substr(0, found);
		QString nameQ = QString::fromStdString(name);
		thisSetting.setValue(defaultPath, crtDir.absoluteFilePath(nameQ));
		m_presetPath = nameQ;
	}

	//////////////////////////////////////////////////////////////////////////
	if (m_useStudyPresets.size() > 0)
	{
		ui.actionEnter_User_Name->setEnabled(true);
	}
}

void OACreater::EnterUserName()
{
	static const QString defaultPath("DEFAULT_USER_NAME_PATH");
	QSettings thisSetting;
	QString path = thisSetting.value(defaultPath).toString();
	 m_userName = QFileDialog::getExistingDirectory(this, tr("Enter user name"), path, QFileDialog::ShowDirsOnly);
	std::string name = m_userName.toStdString();
	m_userStudyDir = name + "\\StudyOne";
	mkdir(&m_userStudyDir[0]);
	//ui.toolBar_User_Study->show();
	if (m_useStudyPresets.size() > 0)
		ui.actionBegin_User_Study_One->setEnabled(true);
	std::cout << name << std::endl;
	size_t found = name.find_last_of('\\');
	name = name.substr(0, found);
	QString nameQ = QString::fromStdString(name);
	QDir crtDir;
	thisSetting.setValue(defaultPath, crtDir.absoluteFilePath(nameQ));
}

void OACreater::StartUserStudy()
{
	ui.actionStart_User_Study->setDisabled(true);
	if (m_useStudyPresets.size() == 0)
		return;
	PrepareUserStudyPreset(m_useStudyPresets.front());
	m_useStudyPresets.pop_front();
	if (m_useStudyPresets.size() > 0)
	{
		ui.actionProceed_User_Study->setEnabled(true);
		ui.actionEnd_User_Study->setDisabled(true);
	}
	else
	{
		ui.actionProceed_User_Study->setDisabled(true);
		ui.actionEnd_User_Study->setEnabled(true);
	}
	ui.displaywidget->repaint();
	//////////////////////////////////////////////////////////////////////////
	// write data
	BeginSaveData();
	SetPreviewDate();
}

void OACreater::ProceedUserStudy()
{
	if (m_useStudyPresets.size() == 0)
	{
		ui.actionProceed_User_Study->setDisabled(true);
		ui.actionEnd_User_Study->setEnabled(true);
		return;
	}
	SaveUserResults();
	PrepareUserStudyPreset(m_useStudyPresets.front());
	m_useStudyPresets.pop_front();
	if (m_useStudyPresets.size() > 0)
	{
		ui.actionProceed_User_Study->setEnabled(true);
		ui.actionEnd_User_Study->setDisabled(true);

	}
	else
	{
		ui.actionProceed_User_Study->setDisabled(true);
		ui.actionEnd_User_Study->setEnabled(true);
	}
	// write data
	EndSaveData();
	BeginSaveData();
	SetPreviewDate();
}

void OACreater::EndUserStudy()
{
	ui.actionStart_User_Study->setDisabled(true);
	ui.actionProceed_User_Study->setDisabled(true);
	ui.actionEnd_User_Study->setDisabled(true);
	ui.actionEnter_User_Name->setDisabled(true);
	SaveUserResults();
	//////////////////////////////////////////////////////////////////////////
	// write data
	EndSaveData();
	if (m_useStudyPresets.size() == 0)
		ui.actionFinish_User_Study->setEnabled(true);
	cv::destroyWindow("target");
}

void OACreater::BeginUserStudyOne()
{
	ui.actionBegin_User_Study_One->setDisabled(true);
	cv::Mat tempImg = cv::Mat::zeros(300, 300, CV_8UC3);
	Utils::ShowImg("target", tempImg);
	ui.actionStart_User_Study->setEnabled(true);
}

void OACreater::FinishUserStudy()
{
	ui.actionStart_User_Study->setDisabled(true);
	ui.actionProceed_User_Study->setDisabled(true);
	ui.actionEnd_User_Study->setDisabled(true);
	ui.actionEnter_User_Name->setDisabled(true);
	ui.actionFinish_User_Study->setDisabled(true);
}

void OACreater::ClearFrontDate()
{
	//relation
	ui.displaywidget->m_editRelations.clear();
	ui.displaywidget->m_addRelations.clear();
	ui.displaywidget->m_deleteRelations.clear();
	ui.displaywidget->m_recordShowRelaion.clear();
	ui.displaywidget->m_recordRelations.clear();
	ui.displaywidget->m_optRelations.clear();
	//undo redo
	m_listShapeHandler_history.clear();
	m_listShapeHandler_frontHistory.clear();
	m_listCamera_history.clear();
	m_listCamera_frontHistory.clear();
	m_editRelaion_history.clear();
	m_editRelaion_frontHistory.clear();
	//spacing 
	ui.displaywidget->m_SpacingRelations.clear();
	ui.displaywidget->m_addSpacingRelations.clear();
	ui.displaywidget->m_addSpacingRelationsGroup.clear();
	ui.displaywidget->m_deleteSpacingRelations.clear();
}

void OACreater::LoadUSP2File(char* pathName)
{
	std::fstream fin;
	fin.open(pathName, std::ios::in);
	int numStudy;
	char tempName[200];

	fin >> numStudy;
	fin.getline(tempName, 200);
	m_useStudyPresets.clear();
	std::string thisString;
	for (int i = 0; i < numStudy; ++i)
	{
		CUserStudyPresets thisPreset;
		fin >> thisString;
		thisPreset.targetImgName = thisString;
		fin >> thisString;
		thisPreset.roughLayoutName = thisString;
		fin >> thisString;
		if (thisString == "NotCheckShapeRotation")
			thisPreset.checkShapeRotation = false;
		else
			thisPreset.checkShapeRotation = true;
		fin >> thisString;
		if (thisString == "NotCheckCameraRotation")
			thisPreset.checkCameraRotation = false;
		else
			thisPreset.checkCameraRotation = true;
		fin >> thisString;
		if (thisString == "NotCheckZoom")
			thisPreset.checkZoom = false;
		else
			thisPreset.checkZoom = true;
		//enable
		fin >> thisString;
		if (thisString == "DisableOpen")
			thisPreset.enableOpen = false;
		else
			thisPreset.enableOpen = true;
		fin >> thisString;
		if (thisString == "DisableSave")
			thisPreset.enableSave = false;
		else
			thisPreset.enableSave = true;
		fin >> thisString;
		if (thisString == "DisableCopy")
			thisPreset.enableCopy = false;
		else
			thisPreset.enableCopy = true;
		fin >> thisString;
		if (thisString == "DisablePaste")
			thisPreset.enablePaste = false;
		else
			thisPreset.enablePaste = true;
		fin >> thisString;
		if (thisString == "DisableDelete")
			thisPreset.enableDelete= false;
		else
			thisPreset.enableDelete = true;
		m_useStudyPresets.push_back(thisPreset);
	}
	fin.flush();
	fin.close();
	fin.clear();
}

void OACreater::PrepareUserStudyPreset(CUserStudyPresets& thisPreset)
{
	m_crtUserStudyPreset = thisPreset;
	cv::Mat thisTarget;
	std::string thisTargetImgPath;
	thisTargetImgPath = m_presetPath.toStdString() + "/" + thisPreset.targetImgName;
	thisTarget = cv::imread(thisTargetImgPath);
	Utils::ShowImg("target", thisTarget);
	//加载模型
	std::string roughLayoutPath = m_presetPath.toStdString() + "/" + thisPreset.roughLayoutName;
	LoadWholeModel(roughLayoutPath.c_str());
	//notcheckRotation
	if (thisPreset.checkShapeRotation)
	{
		ui.actionStop_Rotation->setChecked(true);
	}
	else
	{
		ui.actionStop_Rotation->setChecked(false);
	}
	//rotate with camera
	if (thisPreset.checkCameraRotation)
	{
		ui.actionRotate_With_Camera->setChecked(true);
	}
	else
	{
		ui.actionRotate_With_Camera->setChecked(false);
	}
	//zoom
	if (thisPreset.checkZoom)
	{
		ui.actionZoom->setChecked(true);
	}
	else
	{
		ui.actionZoom->setChecked(false);
	}
	//DisableRotation
	//open
	if (thisPreset.enableOpen)
	{
		ui.actionOpen_Shape->setEnabled(true);
	}
	else
	{
		ui.actionOpen_Shape->setDisabled(true);
	}
	//save
	if (thisPreset.enableSave)
	{
		ui.actionSave_Shape->setEnabled(true);
	}
	else
	{
		ui.actionSave_Shape->setDisabled(true);
	}
	//Copy
	if (thisPreset.enableCopy)
	{
		ui.actionCopy->setEnabled(true);
	}
	else
	{
		ui.actionCopy->setDisabled(true);
	}
	//Paste
	if (thisPreset.enablePaste)
	{
		ui.actionPaste->setEnabled(true);
	}
	else
	{
		ui.actionPaste->setDisabled(true);
	}
	//Delete
	if (thisPreset.enableDelete)
	{
		ui.actionDelete->setEnabled(true);
	}
	else
	{
		ui.actionDelete->setDisabled(true);
	}
	ui.displaywidget->setManipulatedFrame(ui.displaywidget->camera()->frame());
}

void OACreater::BeginSaveData()
{
	std::string saveFilePath;
	std::string studyName;
	studyName = m_userStudyDir;
	std::string fileName = m_crtUserStudyPreset.targetImgName;
	size_t found = fileName.find_last_of('\\');
	fileName = fileName.substr(found);
	found = fileName.find_first_of('.');
	fileName = fileName.substr(0, found);
	saveFilePath = studyName + fileName + "_GA.txt";
	//std::string fileName = m_userName.toStdString() + "\\" + QString::number(1).toStdString() + ".txt";
	if (m_writerp != NULL)
	{
		delete m_writerp;
		m_writerp = NULL;
	}
	m_writerp = new std::fstream();
	m_writerp->open(&saveFilePath[0], ios::out);
	ui.displaywidget->SetWriter(m_writerp);
	m_startTime = clock();
	(*m_writerp) << std::setprecision(15);
	(*m_writerp) << "BEGIN " << (double)m_startTime << std::endl;
	m_writerp->flush();
}

void OACreater::SaveUserResults()
{
	std::string saveFilePath, saveFileRefinedPath, saveImgPath, saveImgRefinedPath;
	std::string studyName;
	studyName = m_userStudyDir;
	std::string fileName = m_crtUserStudyPreset.targetImgName;
	size_t found = fileName.find_last_of('\\');
	fileName = fileName.substr(found);
	found = fileName.find_first_of('.');
	fileName = fileName.substr(0, found);
	saveFilePath = studyName + fileName + "_GA.model";
	saveImgPath = studyName + fileName + "_GA.png";
	SaveShapeLayout(&saveFilePath[0]);
	ui.displaywidget->SaveCrtPaintArea(&saveImgPath[0]);
}

void OACreater::EndSaveData()
{
	clock_t endTime = clock();
	(*m_writerp) << "END " << (double)endTime << std::endl;
	(*m_writerp) << "TOTAL_TIME " << (double)(endTime - m_startTime) << std::endl;
	ui.displaywidget->FinishWrite();
	m_writerp->flush();
	m_writerp->close();
	m_writerp->clear();
	delete m_writerp;
	m_writerp = NULL;
}

void OACreater::SaveShapeLayout(char* pathName)
{
	QString filePath = pathName;
	if (!filePath.isEmpty())
	{
		std::string name = filePath.toStdString();
		QDir crtDir;
		std::vector<bool> floatingFlag;
		for (int i = 0; i < m_shapeHandler_p.m_allParts.size();i++)
		{
			floatingFlag.push_back(m_shapeHandler_p.m_allParts[i]->m_isFloating);
		}
		SaveFixAllModels();
		m_shapeHandler_p.SaveWholeModel(name.c_str(),floatingFlag);

		std::string dataDir = name;
		size_t found = dataDir.find_last_of('.');
		dataDir = dataDir.substr(0, found);
		dataDir = dataDir + "_data";
		std::string thisViewPath = dataDir + "/" + "camera.vmat";
		CViewMat thisViewMat;
		thisViewMat = *(ui.displaywidget->camera()->frame());
		thisViewMat.SaveView(thisViewPath.c_str());
		found = name.find_last_of('/');
		name = name.substr(0, found);
		QString nameQ = QString::fromStdString(name);
		SetFloating(floatingFlag);
	}
}

void OACreater::SaveFixAllModels()
{
	for (int i = 0; i < m_shapeHandler_p.m_allParts.size(); i++)
	{
		if (m_shapeHandler_p.m_allParts[i]->m_isFloating)
		{
			m_shapeHandler_p.m_allParts[i]->m_isFloating = false;
			//切换float/fix的时候保持位置不变
			GLdouble matrix[16];
			for (int k = 0; k < 16; k++)
			{
				matrix[k] = m_shapeHandler_p.m_allParts[i]->m_frame->matrix()[k];
			}
			GLdouble camera_matrix[16];
			for (int k = 0; k < 16; k++)
			{
				camera_matrix[k] = ui.displaywidget->camera()->frame()->matrix()[k];
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
			m_shapeHandler_p.m_allParts[i]->m_frame->setFromMatrix(matrix);
			// 设置参考frame为NULL，解除悬浮状态，使其具有位置
			m_shapeHandler_p.m_allParts[i]->m_frame->setReferenceFrame(NULL);
		}
	}
}

void OACreater::OperateCamera()
{
	DisableAllOperationState();
	ui.displaywidget->m_mouseOperateType = SC::OperateCamera;
	ui.actionOperate_Camera->setChecked(true);
	ui.displaywidget->repaint();
	
	
}

void OACreater::Zoom(bool thisFlag)
{
	if (thisFlag)
	{
		ui.displaywidget->m_isZoom = true;
	}
	else
	{
		ui.displaywidget->m_isZoom = false;
	}
	ui.displaywidget->repaint();
}

void OACreater::AddSpacingStart()
{
	DisableAllOperationState();
	ui.actionAdd_Spacing_Start->setChecked(true);
	ui.displaywidget->clearMouseBindings();
	ui.displaywidget->m_mouseOperateType = SC::AddSpacingStart;
	ui.displaywidget->setManipulatedFrame(ui.displaywidget->camera()->frame());
	ui.displaywidget->repaint();
}

void OACreater::AddSpacingEnd()
{
	if (ui.displaywidget->m_addSpacingRelations.size()>1)
	ui.displaywidget->m_addSpacingRelationsGroup.push_back(ui.displaywidget->m_addSpacingRelations);
	ui.displaywidget->m_addSpacingRelations.clear();
}

void OACreater::RemoveSpacing()
{
	DisableAllOperationState();
	ui.actionRemove_Spacing->setChecked(true);
	ui.displaywidget->clearMouseBindings();
	ui.displaywidget->m_mouseOperateType = SC::RemoveSpacing;
	ui.displaywidget->setManipulatedFrame(ui.displaywidget->camera()->frame());
	ui.displaywidget->repaint();
}

void OACreater::ShaowSpacing(bool thisFlag)
{
	ChageMouseToOperateShape();
	if (thisFlag)
	{
		ui.displaywidget->m_isShowSpacingRelations = true;
		ui.displaywidget->UpdateRelations();
	}
	else
	{
		ui.displaywidget->m_isShowSpacingRelations = false;
		ui.displaywidget->m_SpacingRelations.clear();
		ui.displaywidget->m_addSpacingRelationsGroup.clear();
		ui.displaywidget->m_addSpacingRelations.clear();
		ui.displaywidget->m_deleteSpacingRelations.clear();

	}
	ui.displaywidget->repaint();
	
}

void OACreater::ShowGrid(bool thisFlag)
{
	if (thisFlag)
	{
		ui.displaywidget->m_isShowGrid = true;
	}
	else
	{
		ui.displaywidget->m_isShowGrid = false;
	}
	ui.displaywidget->repaint();
}






