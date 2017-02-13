#pragma once

#include <QApplication>
#include "EditorEngine.h"

namespace AM
{
	class EditorApplication : public QApplication
	{
		Q_OBJECT

	public:
		EditorApplication(int& argc, char** argv);
		~EditorApplication();

		void InitApplication();
		void StopApplication();
	private:
		EditorEngine m_editorEngine;
	};

#define qEditorApp static_cast<EditorApplication*>(QCoreApplication::instance())
}
