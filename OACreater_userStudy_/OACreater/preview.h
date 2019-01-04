#pragma once
#include <QGLViewer/qglviewer.h>
#include <QGLViewer/frame.h>
#include <QGLViewer/manipulatedFrame.h>
#include <QGLViewer/manipulatedCameraFrame.h>
#include <QGLViewer/VRender/Vector3.h>
#include <QGLViewer/quaternion.h>
#include <QGLViewer/vec.h>
#include "ShapeHandler.h"
class Preview :
	public QGLViewer
{
	Q_OBJECT
public:
	Preview(QWidget *parent = 0);
	~Preview();
private:
	void Draw3DFuncs();
	void InitializeWidget();
	void InitializeMouseBinding();
	void DrawShapeNo(QPainter& painter);
	void DrawGrid(float size = 1.0f, int nbSubdivisions = 10);
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
public:
	void SetData(CShapeHandler shapeHandlerp);
private:
	CShapeHandler m_shapeHandler;
public:
	bool m_isShowNo;
};

