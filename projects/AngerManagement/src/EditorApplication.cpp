﻿#include "EditorApplication.h"
#include "EditorMainWindow.h"

#include <QThread>
#include <Core/GameWindow.h>
#include <thread>

namespace AM
{
	EditorApplication::EditorApplication(int& argc, char** argv) : QApplication(argc, argv)
	{
	}

	EditorApplication::~EditorApplication()
	{
	}

	void EditorApplication::InitApplication()
	{
		auto topLevelWidgetList = topLevelWidgets();
		EditorMainWindow* mainWindow = nullptr;

		for (auto currentWidgetIter = topLevelWidgetList.begin(); currentWidgetIter != topLevelWidgetList.end(); ++currentWidgetIter)
		{
			if (EditorMainWindow* currentWidget = qobject_cast<EditorMainWindow*>(*currentWidgetIter))
			{
				mainWindow = currentWidget;
				break;
			}
		}

		Q_ASSERT(mainWindow != nullptr);

		// Get the HWND of the panel
		if (!m_editorEngine.InitializeEngine(mainWindow->GetSceneWindowId()))
		{
			return;
		}

		QThread* engineThread = new QThread();
		m_editorEngine.moveToThread(engineThread);

		connect(engineThread, SIGNAL(started()), &m_editorEngine, SLOT(RunEngine()));
		connect(engineThread, SIGNAL(finished()), engineThread, SLOT(deleteLater()));

		engineThread->start();
	}

	void EditorApplication::StopApplication()
	{
		m_editorEngine.StopEngine(); // Tells the engine to stop, but the application only exits when the engine is finished 
	}

}
