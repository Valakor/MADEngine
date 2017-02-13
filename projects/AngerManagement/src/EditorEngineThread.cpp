#include "EditorEngineThread.h"
#include <QCoreApplication>
#include <Core/GameWindow.h>

EditorEngineThread::EditorEngineThread(QObject* parent) : QObject(parent)
{
}

EditorEngineThread::~EditorEngineThread()
{
}

bool EditorEngineThread::InitializeEngine(WId inWindowHandleId)
{
	return m_nativeGameEngine.Init(eastl::make_shared<MAD::UGameWindow>(reinterpret_cast<HWND>(inWindowHandleId)));
}

void EditorEngineThread::RunEngine()
{
	if (m_nativeGameEngine.IsSimulating())
	{
		return;
	}

	m_nativeGameEngine.Run();

	// After the engine is finished running, we can tell the application to quit
	qApp->quit();
}

void EditorEngineThread::StopEngine()
{
	m_nativeGameEngine.Stop();
}
