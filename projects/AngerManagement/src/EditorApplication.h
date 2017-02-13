#pragma once

#include <QApplication>
#include <QProcess>
#include <QTextEdit>

#include "EditorEngineThread.h"

class EditorApplication : public QApplication
{
	Q_OBJECT

public:
	EditorApplication(int& argc, char** argv);
	~EditorApplication();

	void InitApplication();
	void StopApplication();

	class EditorMainWindow* GetMainWindow();
signals:
	void EngineInitFinished();
private:
	EditorEngineThread m_editorEngine;
};

#define qEditorApp static_cast<EditorApplication*>(QCoreApplication::instance())
