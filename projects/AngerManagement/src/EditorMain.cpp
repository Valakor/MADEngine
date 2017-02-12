#include "MainEditorWindow.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	AngerManagementWindow w;
	w.show();
	w.InitEditor();
	return a.exec();
}
