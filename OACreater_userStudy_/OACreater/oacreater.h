#ifndef OACREATER_H
#define OACREATER_H

#include <QtWidgets/QMainWindow>
#include <iostream>
#include <QFileDialog>
#include <QMessageBox>
#include "ShapePart.h"
#include "ShapeHandler.h"
#include "ViewMat.h"
#include "Utils.h"
#include "ui_oacreater.h"

class OACreater : public QMainWindow
{
	Q_OBJECT
public:
	OACreater(QWidget *parent = 0);
	~OACreater();
private slots:
	void OpenShape();
	void SaveShape();
	void Undo();
	void Redo();
	void Copy();
	void Paste();
	void Delete();
	void SelectModel();
	void FixAllModels();
	void FloatAllModels();
	void UpdateRelations(bool);
	void StopRelationUpdate();
	void ApplyRefinement();
	void AddRelaions();
	void ClearEditRelaions();
	void SelectShape();
	void ChageMouseToOperateShape();
	void CurveAlignment();
	void ChangeScale();
	void DrawShapeNo(bool);
	void StopRotation(bool);
	void ShowFourRelaions(bool);
	void RotateWithCamera(bool);
	void OperateCamera();
	void Zoom(bool);
	void AddSpacingStart();
	void AddSpacingEnd();
	void RemoveSpacing();
	void ShaowSpacing(bool);
	void ShowGrid(bool);
	//
	void BackupDataReceiver();
	void SetPreviewDate();
	//user study
	void LoadUserStudyPresets();
	void EnterUserName();
	void StartUserStudy();
	void ProceedUserStudy();
	void EndUserStudy();
	void BeginUserStudyOne();
	void FinishUserStudy();
	
	
private:
	void DisableAllOperationState();
	void AddNewPart(const char* shapePartPath);
	void LoadWholeModel(const char* modelPath);
	void TransferData();
	void UpdateUndoRedoButton();
	bool CompareScene(CShapeHandler shapeHandler1, CShapeHandler shapeHandler2, qglviewer::Frame frame1, qglviewer::Frame frame2);
	//connect
	void ManuallySetupUI();
	void RemoveAndUpdateRelation();
	void SetFloating(std::vector<bool> floatings);
	void ClearEditRelaion();
	void ClearFrontDate();
	void LoadUSP2File(char* pathName);
	void PrepareUserStudyPreset(CUserStudyPresets& thisPreset);
	void BeginSaveData();
	void SaveUserResults();
	void EndSaveData();
	//
	void SaveShapeLayout(char* pathName);
	void SaveFixAllModels();
private:
	Ui::OACreaterClass ui;
	QString m_filePath;
	QString m_appName;
	CShapeHandler m_shapeHandler_p;
	//undo redo
	std::list<CShapeHandler> m_listShapeHandler_history;
	std::list<CShapeHandler> m_listShapeHandler_frontHistory;
	std::list<qglviewer::Frame> m_listCamera_history;
	std::list<qglviewer::Frame> m_listCamera_frontHistory;
	qglviewer::Frame m_frame_backup;
	//edit record opt relaions
	std::list<EditRelaion> m_editRelaion_history;
	std::list<EditRelaion> m_editRelaion_frontHistory;
	//spacing 
	std::list<SpacingRelaion> m_spacingRelation_history;
	std::list<SpacingRelaion> m_spacingRelation_frontHistory;
	//copy paste
	CShapePart m_copiedShape;
	int  m_pasteCount;//同一个shape复制的数量
	int m_shapeId;
	//user study
	std::list<CUserStudyPresets> m_useStudyPresets;
	CUserStudyPresets m_crtUserStudyPreset;
	clock_t m_startTime;
	QString m_fileName;
	QString m_userName;
	QString m_presetPath;
	std::string m_userStudyDir;
	std::fstream* m_writerp;
};
#endif // OACREATER_H
