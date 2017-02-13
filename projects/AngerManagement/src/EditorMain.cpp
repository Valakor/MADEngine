#include "EditorApplication.h"
#include "EditorMainWindow.h"

int main(int argc, char *argv[])
{
	EditorApplication a(argc, argv);
	EditorMainWindow w;

	w.show();

	a.InitApplication();

	return a.exec();
}
