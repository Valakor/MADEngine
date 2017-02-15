#include "EditorEngineProxy.h"
#include <QCoreApplication>

#include <Core/GameWindow.h>
#include <Core/GameWorld.h>

EditorEngineProxy::EditorEngineProxy(QObject* parent) : QObject(parent)
{
}

EditorEngineProxy::~EditorEngineProxy()
{
}

bool EditorEngineProxy::InitializeEngine(WId inWindowHandleId)
{
	return m_editorEngine.Init(eastl::make_shared<MAD::UGameWindow>(reinterpret_cast<HWND>(inWindowHandleId)));
}

eastl::vector<eastl::shared_ptr<MAD::OGameWorld>> EditorEngineProxy::GetGameWorlds() const
{
	std::lock_guard<std::mutex> lockGuard(m_nativeEngineMutex);

	return m_editorEngine.GetWorlds();
}

void EditorEngineProxy::UpdateEntityPosition(eastl::shared_ptr<MAD::AEntity> inEntity, const MAD::Vector3& inNewPosition)
{
	std::lock_guard<std::mutex> lockGuard(m_nativeEngineMutex);

	inEntity->SetWorldTranslation(inNewPosition);
}

void EditorEngineProxy::RunEngine()
{
	if (m_editorEngine.IsSimulating())
	{
		return;
	}

	// ==================== Engine Thread Main Loop ===============================
	// The editor engine will need to lock the engine tick mutex each tick so that the editor and engine are synchronized in terms of resource access and modification (i.e. Entities)
	m_editorEngine.ExecuteEngineTests();

	while (m_editorEngine.IsRunning())
	{
		// Lock guard the engine tick mutex in case the Tick() causes an exception
		std::lock_guard<std::mutex> lockGuard(m_nativeEngineMutex);

		m_editorEngine.Tick();
	}

	m_editorEngine.GetWindow().CaptureCursor(false);

	// After the engine is finished running, we can tell the application to quit
	qApp->quit();
}

void EditorEngineProxy::StopEngine()
{
	m_editorEngine.Stop();
}
