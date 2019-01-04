#include "Utils.h"
//全局变量
//ransac  threshold 
double Utils::gdv_twoEdgAngleThreshold = 0.9;   //0.8-1.0  defaut=0.95
double Utils::gdv_twoEdgeDistanceThreshold = 10;  //10-30		defaut=10
//rotate threshold
float Utils::gdv_RotatingRatio = 0.5;	//default 0.5
int Utils::gdv_MaxIteNumbers = 5;  //default 5
Utils::Utils()
{
}


Utils::~Utils()
{
}
//************************************
// Method:    MatrixMult
// FullName:  Utils::MatrixMult
// Access:    public static 
// Returns:   void
// Qualifier:4*4的矩阵相乘，结果为第一个参数
// Parameter: double * a 矩阵1
// Parameter: double * b 矩阵2
//************************************
void Utils::MatrixMult(double *a, double *b)
{
	cv::Mat mat_m = cv::Mat::zeros(4, 4, CV_64F);
	cv::Mat mat_a = cv::Mat::zeros(4, 4, CV_64F);
	cv::Mat mat_b = cv::Mat::zeros(4, 4, CV_64F);
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			mat_a.at<double>(i, j) = a[i * 4 + j];
			mat_b.at<double>(i, j) = b[i * 4 + j];
		}
	}
	mat_m = mat_b.t()*mat_a.t();
	mat_m = mat_m.t();
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			a[i * 4 + j] = mat_m.at<double>(i, j);

		}
	}
}

cv::Vec3d Utils::Nomalize(cv::Vec3d Pt3D)
{
	float dis = sqrt(Pt3D[0] * Pt3D[0] + Pt3D[1] * Pt3D[1] + Pt3D[2] * Pt3D[2]);
	Pt3D[0] = Pt3D[0] / dis;
	Pt3D[1] = Pt3D[1] / dis;
	Pt3D[2] = Pt3D[2] / dis;
	return Pt3D;

}

cv::Vec2d Utils::Nomalize(cv::Vec2d Pt2D)
{
	float dis = sqrt(abs(Pt2D[0] * Pt2D[0] + Pt2D[1] * Pt2D[1]));
	Pt2D[0] = Pt2D[0] / dis;
	Pt2D[1] = Pt2D[1] / dis;
	return Pt2D;
}

float Utils::Calculate2DTwoPointDistance(cv::Vec2d *point)
{
	return sqrt((point[1][0] - point[0][0])*(point[1][0] - point[0][0]) + (point[1][1] - point[0][1])*(point[1][1] - point[0][1]));
}

float Utils::Calculate2DTwoPointDistance(cv::Vec2d point1, cv::Vec2d point2)
{
	return sqrt((point1[0] - point2[0])*(point1[0] - point2[0]) + (point1[1] - point2[1])*(point1[1] - point2[1]));
}

float Utils::Calculate2DTwoPointDistance(cv::Vec3d point1, cv::Vec3d point2)
{
	return sqrt((point1[0] - point2[0])*(point1[0] - point2[0]) + (point1[1] - point2[1])*(point1[1] - point2[1]) + (point1[2] - point2[2])*(point1[2] - point2[2]));
}

//************************************
// Method:    CalculatePointToLineDistance
// FullName:  Utils::CalculatePointToLineDistance
// Access:    public static 
// Returns:   float
// Qualifier: 计算点到线的距离
// Parameter: cv::Vec2d dir 线的方向
// Parameter: cv::Vec2d point 线上的点
// Parameter: cv::Vec2d point1 线外的点
//************************************
float Utils::CalculatePointToLineDistance(cv::Vec2d dir, cv::Vec2d point, cv::Vec2d point1)
{
	cv::Vec2d vec;
	vec[0] = point1[0] - point[0];
	vec[1] = point1[1] - point[1];
	float dis =vec.dot(dir);
	float d = sqrt(abs(vec[0] * vec[0] + vec[1] * vec[1]));
	return sqrt(abs(d*d - dis*dis));
}

//************************************
// Method:    FitLine2D
// FullName:  Utils::FitLine2D
// Access:    private static 
// Returns:   int
// Qualifier: 最小二乘法拟合直线
// Parameter: Point2D32f * points
// Parameter: int count
// Parameter: float * line
//************************************
int Utils::FitLine2D(Point2D32f * points, int count, float *line)
{
	//进行一遍拟合，获取直线参数
	FitLine2D(points, count, NULL, line);
	//计算权值，再进行一次拟合
	float *dist = new float[count];
	float *W = new float[count];

	//迭代进行加权拟合，迭代次数不小于三次
	for (int i = 0; i < 5; i++)
	{
		CalcDist2D(points, count, line, dist);
		WeightL1(dist, count, W);
		FitLine2D(points, count, W, line);
	}
	delete[] dist;
	return 0;
}

int Utils::FitLine2D(Point2D32f * points, int _count, float *weights, float *line)
{
	double x = 0, y = 0, x2 = 0, y2 = 0, xy = 0, w = 0;
	double dx2, dy2, dxy;
	int i;
	int count = _count;

	float t;

	/* Calculating the average of x and y... */
	if (weights == 0)
	{
		for (i = 0; i < count; i += 1)
		{
			x += points[i].x;
			y += points[i].y;
			x2 += points[i].x * points[i].x;
			y2 += points[i].y * points[i].y;
			xy += points[i].x * points[i].y;
		}
		w = (float)count;
	}
	else
	{
		for (i = 0; i < count; i += 1)
		{
			x += weights[i] * points[i].x;
			y += weights[i] * points[i].y;
			x2 += weights[i] * points[i].x * points[i].x;
			y2 += weights[i] * points[i].y * points[i].y;
			xy += weights[i] * points[i].x * points[i].y;
			w += weights[i];
		}
	}

	x /= w;
	y /= w;
	x2 /= w;
	y2 /= w;
	xy /= w;

	dx2 = x2 - x * x;
	dy2 = y2 - y * y;
	dxy = xy - x * y;

	t = (float)atan2(2 * dxy, dx2 - dy2) / 2;
	line[0] = (float)cos(t);
	line[1] = (float)sin(t);
	line[2] = (float)x;
	line[3] = (float)y;

	return 0;
}

double Utils::CalcDist2D(Point2D32f * points, int count, float *_line, float *dist)
{
	int j;
	float px = _line[2], py = _line[3];
	float nx = _line[1], ny = -_line[0];
	double sum_dist = 0.;

	for (j = 0; j < count; j++)
	{
		float x, y;

		x = points[j].x - px;
		y = points[j].y - py;

		dist[j] = (float)fabs(nx * x + ny * y);
		sum_dist += dist[j];
	}

	return sum_dist;
}

void Utils::WeightL1(float *d, int count, float *w)
{
	int i;

	for (i = 0; i < count; i++)
	{
		double t = fabs((double)d[i]);
		w[i] = (float)(1. / max(t, gdc_eps));
	}
}

void Utils::quat2angle(std::vector<float> &angle, std::vector<float> &quaternion)
{
	float x, y, z, w;
	x = quaternion[0];
	y = quaternion[1];
	z = quaternion[2];
	w = quaternion[3];
	angle[0] = atan2(2 * (w*x + y*z), 1 - 2 * (x*x + y*y));
	angle[1] = asin(2 * (w*y - x*z));
	angle[2] = atan2(2 * (w*z + y*x), 1 - 2 * (y*y + z*z));
}

double Utils::max(double a, double b)
{
	return a > b ? a : b;
}

bool Utils::compare2Vec(cv::Vec2d v1, cv::Vec2d v2)
{
	if (v1[0] == v2[0] && v1[1] == v2[1])
	{
		return true;
	}
	return false;
}

//************************************
// Method:    ClosestPointOnSegment
// FullName:  Utils::ClosestPointOnSegment
// Access:    public static 
// Returns:   cv::Vec3d 投影点
// Qualifier:计算线段外的一点到线段的投影点
// Parameter: cv::Vec3d p 线段外的一点
// Parameter: cv::Vec3d p1 线段上的一个端点
// Parameter: cv::Vec3d p2 线段上的另一个端点
//************************************
cv::Vec3d Utils::ClosestPointOnSegment(cv::Vec3d p, cv::Vec3d p1, cv::Vec3d p2)
{
	cv::Vec3d diff = p - p1;
	cv::Vec3d dir = p2 - p1;
	float dot1 = diff.dot(dir);
	if (dot1 <= 0.0f) {
		return p1;
	}
	float dot2 = dir.dot(dir);
	if (dot2 <= dot1) {
		return p2;
	}
	float t = dot1 / dot2;
	return p1 + t * dir;
}

//************************************
// Method:    PointToSegmentMinDistance
// FullName:  Utils::PointToSegmentMinDistance
// Access:    public static 
// Returns:   double 距离
// Qualifier:直线外的一点到线段的最短距离
// Parameter: cv::Vec3d p 线段外的一点
// Parameter: cv::Vec3d p1 线段的一个端点
// Parameter: cv::Vec3d p2 线段的一个端点
//************************************
double Utils::PointToSegmentMinDistance(cv::Vec2d p, cv::Vec2d p1, cv::Vec2d p2)
{
	//Utils::Calculate2DTwoPointDistance(p,p1);
	cv::Vec2d a, b, c;
	a = p1; b = p2; c = p;
	cv::Vec2d   ab = b - a;
	cv::Vec2d  ac = c - a;
	float f = ab.dot(ac);
	if (f < 0) return Calculate2DTwoPointDistance(a, c);
	float d = ab.dot(ab);
	if (f > d) return Calculate2DTwoPointDistance(b, c);
	f = f / d;
	cv::Vec2d  D = a + f *ab;   // c在ab线段上的投影点
	return Calculate2DTwoPointDistance(c, D);
}

cv::Vec3d Utils::pointToLineProjection(cv::Vec3d linePoint1, cv::Vec3d linePoint2, cv::Vec3d point3)
{
	cv::Vec3d projectionPos(0, 0, 0);
	cv::Vec3d v1 = point3 - linePoint1;
	cv::Vec3d v2 = linePoint2 - linePoint1;
	double dis = Calculate2DTwoPointDistance(linePoint2, linePoint1);
	if (dis*dis == 0)
		return projectionPos;
	double k = (v1.dot(v2)) / (dis*dis);
	projectionPos = v2*k + linePoint1;
	return projectionPos;


}

cv::Vec2d Utils::pointToLineProjection(cv::Vec2d linePoint1, cv::Vec2d linePoint2, cv::Vec2d point3)
{
	cv::Vec2d projectionPos(0, 0);
	cv::Vec2d v1 = point3 - linePoint1;
	cv::Vec2d v2 = linePoint2 - linePoint1;
	double dis = Calculate2DTwoPointDistance(linePoint2, linePoint1);
	if (dis*dis == 0)
		return projectionPos;
	double k = (v1.dot(v2)) / (dis*dis);
	projectionPos = v2*k + linePoint1;
	return projectionPos;
}

cv::Vec3d Utils::CalculateCrossPoint(cv::Vec3d u1, cv::Vec3d u2, cv::Vec3d v1, cv::Vec3d v2)
{
	cv::Vec3d crossPoint;
	cv::Vec3d dir1 = u2 - u1;
	cv::Vec3d dir2 = v2 - v1;
	//单位化
	dir1 = Nomalize(dir1);
	dir2 = Nomalize(dir2);
	if (dir1.cross(dir2) == cv::Vec3d(0, 0, 0))
		return cv::Vec3d(0, 0, 0);
	if (GetVecLenth(dir2.cross(dir1)) == 0)
		return cv::Vec3d(0, 0, 0);
	double t1 = GetVecLenth((u1 - v1).cross(dir2)) / GetVecLenth(dir2.cross(dir1));
	crossPoint = u1 + t1*dir1;
	return crossPoint;
}

double Utils::GetVecLenth(cv::Vec3d v)
{
	return sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

bool Utils::CompareTwoRecordNo(RecordNo r1, RecordNo r2)
{
	if (r1.edge_no1==r2.edge_no1&&r1.edge_no2==r2.edge_no2&&r1.shape_no1==r2.shape_no1&&r1.shape_no2==r2.shape_no2&&r1.vertices_no1==r2.vertices_no1&&r1.vertices_no2==r2.vertices_no2)
	{
		return true;
	}
	if (r1.edge_no1 == r2.edge_no2&&r1.edge_no2 == r2.edge_no1&&r1.shape_no1 == r2.shape_no2&&r1.shape_no2 == r2.shape_no1&&r1.vertices_no1 == r2.vertices_no2&&r1.vertices_no2 == r2.vertices_no1)
	{
		return true;
	}
	return false;
}

cv::Vec3d Utils::MatrixMult31(double* a, cv::Vec3d dir)
{
	cv::Vec3d result;
	cv::Mat mat_a = cv::Mat::zeros(4, 4, CV_64F);
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			mat_a.at<double>(i, j) = a[i * 4 + j];
		}
	}
	//mat_a = mat_a.t();
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			a[i * 4 + j] = mat_a.at<double>(i, j);
		}
	}
	result[0] = a[0] * dir[0] + a[1] * dir[1] + a[2] * dir[2] + a[3];
	result[1] = a[4] * dir[0] + a[5] * dir[1] + a[6] * dir[2] + a[7];
	result[2] = a[8] * dir[0] + a[9] * dir[1] + a[10] * dir[2] + a[11];
	double w = a[12] * dir[0] + a[13] * dir[1] + a[14] * dir[2] + a[15];
	result[0] = result[0] / w;
	result[1] = result[1] / w;
	result[2] = result[2] / w;
	return result;
}

void Utils::MatrixMult44(double *a, double *b)
{
	cv::Mat mat_m = cv::Mat::zeros(4, 4, CV_64F);
	cv::Mat mat_a = cv::Mat::zeros(4, 4, CV_64F);
	cv::Mat mat_b = cv::Mat::zeros(4, 4, CV_64F);
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			mat_a.at<double>(i, j) = a[i * 4 + j];
			mat_b.at<double>(i, j) = b[i * 4 + j];
		}
	}
	//mat_m = mat_a*mat_b.t();
	mat_m = mat_b.t()*mat_a;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			a[i * 4 + j] = mat_m.at<double>(i, j);

		}
	}
}

void Utils::ShowImg(std::string windowName, cv::Mat& img)
{
	cv::namedWindow(windowName);
	cv::imshow(windowName, img);
}







