#pragma once

#include <QApplication>

namespace AM
{
	class EditorApplication : public QApplication
	{
		Q_OBJECT

	public:
		EditorApplication(int& argc, char** argv);
		~EditorApplication();

	private:
		
	};
}
