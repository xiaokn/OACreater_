#include "oacreater.h"
#include <QtWidgets/QApplication>
#include "LinkLibs.h"

int main(int argc, char *argv[])
{
	QCoreApplication::setOrganizationName("XXX");
	QCoreApplication::setApplicationName("File Path");
	QCoreApplication::setApplicationVersion("1.0.0");
	QApplication a(argc, argv);
	OACreater w;
	w.setWindowTitle("3DOA");
	w.show();
	return a.exec();
}
