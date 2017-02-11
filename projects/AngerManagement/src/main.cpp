#include "angermanagement.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	AngerManagement w;
	w.show();
	return a.exec();
}
