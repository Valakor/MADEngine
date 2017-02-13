#include "EditorEngine.h"
#include <QCoreApplication>
#include <Core/GameWindow.h>

EditorEngine::EditorEngine(QObject* parent) : QObject(parent)
{
}

EditorEngine::~EditorEngine()
{
}

bool EditorEngine::InitializeEngine(WId inWindowHandleId)
{
	return m_nativeGameEngine.Init(eastl::make_shared<MAD::UGameWindow>(reinterpret_cast<HWND>(inWindowHandleId)));
}

void EditorEngine::RunEngine()
{
	m_nativeGameEngine.Run();

	// After the engine is finished running, we can tell the application to quit
	qApp->quit();
}

void EditorEngine::StopEngine()
{
	m_nativeGameEngine.Stop();
}

