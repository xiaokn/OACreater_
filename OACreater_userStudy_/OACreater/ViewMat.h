#pragma once
#include <fstream>
#include <cassert>
#include <QGLViewer/quaternion.h>
#include <QGLViewer/frame.h>
#include <QGLViewer/manipulatedFrame.h>
class CViewMat
{
public:
	CViewMat();
	~CViewMat();
public:
	double& operator()(int i, int j);
	double& operator[](int i);
	CViewMat& operator= (double m[16]);
	CViewMat& operator= (double m[4][4]);
	CViewMat& operator= (const qglviewer::Frame& frame);
	void GetRotationMat(float m[3][3]) const;
	void LoadView(const char* viewPath);
	void SaveView(const char* viewPath);
	void GetViewMat(double m[16]);
	qglviewer::Quaternion Orientation();
private:
	double m_m[16];
	qglviewer::Quaternion m_quaternion;

};

