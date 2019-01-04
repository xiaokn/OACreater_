#include "Preview.h"
Preview::Preview(QWidget *parent)
	: QGLViewer(QGLFormat(QGL::SampleBuffers), parent)
{
	InitializeWidget();
	InitializeMouseBinding();
	m_isShowNo = true;
}


Preview::~Preview()
{
}

void Preview::draw()
{
	//DrawGrid(4, 10);
	DrawGrid(2, 3);//4 10
	Draw3DFuncs();
}

void Preview::paintEvent(QPaintEvent* e)
{
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
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_MULTISAMPLE);
	glShadeModel(GL_FLAT);         // Set Smooth Shading                 
	glClearDepth(1.0f);                 // Depth buffer setup             
	//glEnable(GL_DEPTH_TEST);         // Enables Depth Testing             
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
	m_shapeHandler.ComputeEdgeAndNormal(this->camera());//计算边和法线
	//painter.drawText(5, 15, "Preview");
	if (m_isShowNo)
		DrawShapeNo(painter);
	painter.end();
}

void Preview::paintGL()
{
	update();
}

QString Preview::helpString() const
{
	QString text("<h2>OACreater</h2>");
	return text;
}

void Preview::keyPressEvent(QKeyEvent* e)
{
	QGLViewer::keyPressEvent(e);
}

void Preview::mousePressEvent(QMouseEvent* e)
{
	
	QGLViewer::mousePressEvent(e);
}

void Preview::mouseDoubleClickEvent(QMouseEvent* e)
{

}

void Preview::mouseMoveEvent(QMouseEvent* e)
{
	QGLViewer::mouseMoveEvent(e);
}

void Preview::mouseReleaseEvent(QMouseEvent* e)
{
	QGLViewer::mouseReleaseEvent(e);
}

void Preview::wheelEvent(QWheelEvent* e)
{
	qglviewer::ManipulatedCameraFrame* cameraFrame = camera()->frame();
	setManipulatedFrame(cameraFrame);
	setWheelBinding(Qt::NoModifier, QGLViewer::FRAME, QGLViewer::ZOOM);
	QGLViewer::wheelEvent(e);
}

void Preview::resizeEvent(QResizeEvent* e)
{
	QGLViewer::resizeEvent(e);
}

void Preview::drawWithNames()
{
	m_shapeHandler.DrawAllPartsWithNames();
}

void Preview::postSelection(const QPoint& point)
{
	int selectedPartIdx = selectedName();
	m_shapeHandler.SetActivePart(selectedPartIdx);
}

void Preview::Draw3DFuncs()
{
	m_shapeHandler.DrawAllParts(camera());
}

void Preview::InitializeWidget()
{
	setManipulatedFrame(camera()->frame());
	camera()->frame()->setSpinningSensitivity(1000.0);
	camera()->setSceneRadius(camera()->sceneRadius() * 15);
	camera()->setPosition(camera()->position() * 3);
	camera()->setType(qglviewer::Camera::ORTHOGRAPHIC);
	setStateFileName(QString::null);
	camera()->frame()->setZoomSensitivity(2);
}

void Preview::InitializeMouseBinding()
{
	clearMouseBindings();
	setMouseBinding(Qt::NoModifier, Qt::RightButton, QGLViewer::FRAME, QGLViewer::ROTATE);
	setMouseBinding(Qt::NoModifier, Qt::LeftButton, QGLViewer::FRAME, QGLViewer::TRANSLATE);
	setWheelBinding(Qt::NoModifier, QGLViewer::FRAME, QGLViewer::ZOOM);
}

void Preview::SetData(CShapeHandler shapeHandlerp)
{
	m_shapeHandler.DestoryAllParts();
	for (int i = 0; i < shapeHandlerp.m_allParts.size();i++)
	{
		CShapePart* newPart=new CShapePart(*(shapeHandlerp.m_allParts[i]));
		if (!newPart->m_isFloating)
		m_shapeHandler.m_allParts.push_back(newPart);
	}
	repaint();
}

void Preview::DrawShapeNo(QPainter& painter)
{
	for (int i = 0; i < m_shapeHandler.m_allParts.size(); i++)
	{
		m_shapeHandler.m_allParts[i]->DrawNo(painter);
	}
}

void Preview::DrawGrid(float size /*= 1.0f*/, int nbSubdivisions /*= 10*/)
{
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	glEnable(GL_MULTISAMPLE);
	glPushMatrix();
	qglviewer::Quaternion thisQuaternion(qglviewer::Vec(1, 0, 0), M_PI * 0.5);
	glMultMatrixd(thisQuaternion.matrix());
	glColor3d(0.1, 0.1, 0.1);
	//glColor4ub(180,180,180,255);
	//glColor4ub(50, 50, 50, 255);
	QGLViewer::drawGrid(size, nbSubdivisions);
	glPopMatrix();
}

void Preview::init()
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glEnable(GL_LIGHT0);
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
}

