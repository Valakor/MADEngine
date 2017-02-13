#include "EditorApplication.h"
#include "EditorMainWindow.h"

int main(int argc, char *argv[])
{
	AM::EditorApplication a(argc, argv);
	AM::EditorMainWindow w;

	w.show();

	a.InitApplication();

	return a.exec();
}
