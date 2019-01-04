#ifndef UTILS_H
#define UTILS_H
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "globalVal.h"
//一些结构体
typedef struct {
	float x;
	float y;
	int flag;
}Point2D32f;
struct RecordNo
{
	int shape_no1;
	int vertices_no1;
	int edge_no1;
	int shape_no2;
	int vertices_no2;
	int edge_no2;
};
struct SharpConer
{
public:
	cv::Vec3d pt3D;
	cv::Vec2d pt2D;
	bool m_lbv_isVisible;
};
//四个顶点一个面
struct Face
{
	int vertexIndex[4];
	cv::Vec3d normal;
	cv::Vec2d centerPoint; //缩放可视化的点
	cv::Vec3d center3DPoint; //对应的3D点
};
struct CUserStudyPresets
{
public:
	std::string targetImgName;//targetImg
	std::string roughLayoutName;//layout name
	bool checkShapeRotation;
	bool checkCameraRotation;
	bool checkZoom;
	bool enableOpen;
	bool enableSave;
	bool enableCopy;
	bool enablePaste;
	bool enableDelete;

};
static const double gdc_eps = 1e-6;
class Utils
{
public:
	Utils();
	~Utils();
public:
	static double gdv_twoEdgeDistanceThreshold;
	static double gdv_twoEdgAngleThreshold;
	static int  gdv_MaxIteNumbers;
	static float gdv_RotatingRatio;
public:
	static void MatrixMult(double *a, double *b);
	static cv::Vec3d Nomalize(cv::Vec3d Pt3D);
	static cv::Vec2d Nomalize(cv::Vec2d Pt2D);
	static float Calculate2DTwoPointDistance(cv::Vec2d *point);
	static float Calculate2DTwoPointDistance(cv::Vec2d point1, cv::Vec2d point2);
	static float Calculate2DTwoPointDistance(cv::Vec3d point1, cv::Vec3d point2);
	static float CalculatePointToLineDistance(cv::Vec2d dir, cv::Vec2d point, cv::Vec2d point1);
	static int FitLine2D(Point2D32f * points, int count, float *line);
	static void quat2angle(std::vector<float> &angle, std::vector<float> &quaternion);
	static bool compare2Vec(cv::Vec2d v1, cv::Vec2d v2);
	static cv::Vec3d ClosestPointOnSegment(cv::Vec3d p, cv::Vec3d p1, cv::Vec3d p2);
	static double PointToSegmentMinDistance(cv::Vec2d p, cv::Vec2d p1, cv::Vec2d p2);
	static cv::Vec3d pointToLineProjection(cv::Vec3d linePoint1, cv::Vec3d linePoint2, cv::Vec3d point3);
	static cv::Vec2d pointToLineProjection(cv::Vec2d linePoint1, cv::Vec2d linePoint2, cv::Vec2d point3);
	static cv::Vec3d CalculateCrossPoint(cv::Vec3d u1, cv::Vec3d u2, cv::Vec3d v1, cv::Vec3d v2);
	static double GetVecLenth(cv::Vec3d v);
	static bool CompareTwoRecordNo(RecordNo r1, RecordNo r2);
	static cv::Vec3d MatrixMult31(double* a, cv::Vec3d dir);
	static void MatrixMult44(double *a, double *b);
	static void ShowImg(std::string windowName, cv::Mat& img);
private:
	static int FitLine2D(Point2D32f * points, int _count, float *weights, float *line);
	static double CalcDist2D(Point2D32f * points, int count, float *_line, float *dist);
	static void WeightL1(float *d, int count, float *w);
	static double max(double a, double b);
};
namespace SC
{
	enum MouseOperateType{ OperateShape, OperateCamera, RecSelect, AddRelations, DeleteRelations, CurveAlignment, ChangeScale,AddSpacingStart,RemoveSpacing};
}

#endif // UTILS_H
