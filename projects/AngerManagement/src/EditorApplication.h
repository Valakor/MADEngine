#pragma once

#include <QApplication>
#include <QProcess>
#include <QTextEdit>

#include "Engine/EditorEngineThread.h"

class EditorApplication : public QApplication
{
	Q_OBJECT

public:
	EditorApplication(int& argc, char** argv);
	~EditorApplication();

	void InitApplication();
	void StopApplication();

	EditorEngineThread& GetEngineProxy() { return m_editorEngineProxy; }
	class EditorMainWindow* GetMainWindow();
signals:
	void EngineInitFinished();
private:
	EditorEngineThread m_editorEngineProxy;
};

#define qEditorApp static_cast<EditorApplication*>(QCoreApplication::instance())
#define qNativeEngine qEditorApp->GetEngineProxy()
