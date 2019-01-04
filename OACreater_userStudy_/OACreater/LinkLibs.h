#pragma one
#ifdef _DEBUG
# pragma comment(lib, "QGLViewerd2.lib")
# pragma comment(lib, "trimeshd.lib")
# pragma comment(lib, "opencv_core249d.lib")
# pragma comment(lib, "opencv_highgui249d.lib")
# pragma comment(lib, "opencv_imgproc249d.lib")
# pragma comment(lib, "opencv_ml249d.lib")
# pragma comment(lib, "opencv_imgproc249d.lib")
# pragma comment(lib, "LibCholmod32d.lib")
#else
# pragma comment(lib, "trimesh.lib")
# pragma comment(lib, "QGLViewer2.lib")
# pragma comment(lib, "opencv_core249.lib")
# pragma comment(lib, "opencv_highgui249.lib")
# pragma comment(lib, "opencv_imgproc249.lib")
# pragma comment(lib, "opencv_ml249.lib")
# pragma comment(lib, "opencv_imgproc249.lib")
# pragma comment(lib, "LibCholmod32.lib")
#endif