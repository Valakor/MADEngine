#include "EditorApplication.h"
#include "EditorMainWindow.h"

int main(int argc, char *argv[])
{
	AM::EditorApplication a(argc, argv);
	AM::EditorMainWindow w;

	w.show();
	w.InitEditor();
	return a.exec();
}
