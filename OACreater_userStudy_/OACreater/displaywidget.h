#pragma once
#include <QGLViewer/qglviewer.h>
#include <QGLViewer/frame.h>
#include <QGLViewer/manipulatedFrame.h>
#include <QGLViewer/manipulatedCameraFrame.h>
#include <QGLViewer/VRender/Vector3.h>
#include <QGLViewer/quaternion.h>
#include <QGLViewer/vec.h>
#include <QMessageBox>
#include "ShapeHandler.h"
#include "LayoutRefiner.h"
class CDisplayWidget : public QGLViewer
{
	Q_OBJECT
public:
	CDisplayWidget(QWidget *parent = 0);
	~CDisplayWidget();
	void InitializeMouseBinding();
	void UpdateManipulatedFrame();
	void InitializeData();
	void InitializeWidget();
signals:
	void BackupDataSender();
	void FixAllModelsSender();
	void UpdateRelationsSender();
protected:
	virtual void draw();
	virtual void paintEvent(QPaintEvent* e);
	virtual void paintGL();
	virtual void fastDraw() {}
	virtual QString helpString() const;
	virtual void keyPressEvent(QKeyEvent* e);
	virtual void mousePressEvent(QMouseEvent* e);
	virtual void mouseDoubleClickEvent(QMouseEvent* e);
	virtual void mouseMoveEvent(QMouseEvent* e);
	virtual void mouseReleaseEvent(QMouseEvent* e);
	virtual void wheelEvent(QWheelEvent* e);
	virtual void resizeEvent(QResizeEvent* e);
	virtual void drawWithNames();
	virtual void postSelection(const QPoint& point);
	//init light
	virtual void init();
private:
	//一些显示函数
	void Draw3DFuncs();	//显示handler里面所有的shape
	void DrawOtherFunction(QPainter& painter);
	void DrawScaleFaceCenterPoint(QPainter& painter);
	void ScaleShape(CShapePart *shapePart, cv::Vec3d startPoint, cv::Vec2d endPoint);
	cv::Vec2d ComputeProjectedVertex(qglviewer::ManipulatedFrame* frame, cv::Vec3d Pt3D);
	void CloseToCurve();
	void DrawCloseCurveStroke(QPainter& painter);
	void EditSelectShape();
	void DrawSelectRect(QPainter& painter);
	void DrawEditRelationsStroke(QPainter& painter);
	void DrawSpacingStroke(QPainter& painter);
	//计算关系
	void ComputeRelationsByRansac(CShapeHandler* handler);
	void DrawRelitions(QPainter& painter);
	void DrawSpacings(QPainter& painter);
	void DrawEditRelitions(QPainter& painter);
	void RemoveErrorRelaions(CShapeHandler handler);
	bool RelaionIsTrue(CShapeHandler handler);
	void FixModelsByFloatIndex(vector<bool> floatIndex, CShapeHandler* handler);
	void DrawShapeNo(QPainter& painter);
	void DrawGrid(float size = 1.0f, int nbSubdivisions = 10);
	//edit relations
	void AddRelation();
	void DeleteRelation();
	//比较两个关系是否一样
	bool TwoRelaionIsEqual(ShapeRelation relation1, ShapeRelation relation2);
	bool TwoSpacingRelaionIsEqual(RecordNo no1, RecordNo no2);
public:
	//handle的一些数据处理函数
	void DestoryData();
	void SetData(CShapeHandler* shapeHandlerp);
	void UpdateRelations();	//更新关系
	void ExecuteAlignmentOptimizing();//执行优化
	CShapeHandler GetPreviewResult();
	void EditRelations();//用于编辑关系
	void SortRelaions();
	void AddAndDeleteRelaions();
	void GetFinalOptRelaions();
	void SetWriter(std::fstream* writerp);
	void SaveCrtPaintArea(char* imgPath);
	void FinishWrite();
	void GetSpacingRelations(std::vector<ShapeRelation> relaions);
	std::vector<std::vector<RecordNo>> GetSpacingRelations(std::vector<std::vector<ShapeRelation>> group);
	double ComputeTwoEdgeDis(Edge e1,Edge e2);
	vector<ShapeRelation> UpdateVecRelations(vector<ShapeRelation> availableRelations);
	std::vector<std::vector<ShapeRelation>> UpdateVecRelations(std::vector<std::vector<ShapeRelation>> group);
	vector<ShapeRelation> UpdateVecRelations(vector<ShapeRelation> availableRelations,CShapeHandler* handler);
	bool TwoRecordNoisEqual(std::vector<RecordNo> no1, std::vector<RecordNo> no2);
	bool TwoRecordNoisContain(std::vector<RecordNo> no1, std::vector<RecordNo> no2);
	std::vector<RecordNo> SortRecordNo(std::vector<RecordNo> recordNos, CShapeHandler* handler);
	std::vector<RecordNo> RuleOrdering(std::vector<RecordNo> recordNos);
	RecordNo SwapRecordNo(RecordNo recordNo, cv::Vec2d dir);
	void ComputeSpacingRelations();//得到spacing关系
	void AddSpacingRelations();
	void RemoveSpacingRelations();
	void GetFinalSpacingRelaions(std::vector<std::vector<RecordNo>>);
private:
	CShapeHandler* m_shapeHandler_h;
	cv::Vec2d m_startPoint;//鼠标按下的点
	cv::Vec2d m_endPoint;//鼠标释放的点

	//缩放
	std::vector<cv::Vec3d> m_recordCenterPoints;
	cv::Vec3d  m_scaleStartPoint;
	std::vector<QRect> m_selectRect;
	double m_scaleCount;
	cv::Vec3d m_recordFrontProPoint;
	cv::Vec2d m_selectShapePoint[2];		//记录矩形的点
	
public:
	CShapePart *m_shapePart;//当前操作的shape
	SC::MouseOperateType m_mouseOperateType;	//鼠标操作类型
	std::vector<cv::Vec2d> m_curveStrokeAL;		//记录曲线的路径
	std::vector<cv::Vec2d> m_spacingStrokeAL;		//记录曲线的路径
	std::vector<cv::Vec2d> m_relationStrokeAL;		//记录画的路径
	CShapeHandler m_partShapeHandler_h;
	//记录两个点，用于移动时保持关系不变可视化
	std::vector<ShapeRelation> m_recordRelations;//记录关系，用于可视化显示以及关系处理
	std::vector<ShapeRelation> m_optRelations;//可利用的关系，用于优化
	bool m_stopRelationUpdate;
	bool m_operateShape;
	bool m_isShowNo;
	std::vector<ShapeRelation> m_editRelations;//可利用的关系，用于优化
	std::vector<ShapeRelation> m_addRelations;
	std::vector<ShapeRelation> m_deleteRelations;
	std::vector<ShapeRelation> m_recordShowRelaion;
	bool m_isClearMouseBinding;
	bool m_stopShapeRotation;
	//userStudy
	std::fstream* m_writerp;
	bool m_isMoveShape;
	bool m_isEditRelaion;
	int m_editRelationType;
	bool m_isZoom;
	//
	std::vector<std::vector<RecordNo>> m_SpacingRelations;
	std::vector<ShapeRelation> m_addSpacingRelations;
	std::vector<std::vector<ShapeRelation>> m_addSpacingRelationsGroup;
	std::vector<RecordNo> m_deleteSpacingRelations;
	bool m_isShowSpacingRelations;
	bool m_isShowGrid;
};

