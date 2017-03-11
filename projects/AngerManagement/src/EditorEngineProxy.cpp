#include "EditorEngineProxy.h"
#include <QCoreApplication>
#include <QMutexLocker>

#include <Core/GameWindow.h>
#include <Rendering/Renderer.h>
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

void EditorEngineProxy::OnWindowSizeChanged()
{
	QMutexLocker lockGuard(&m_nativeEngineMutex);

	m_editorEngine.GetRenderer().OnScreenSizeChanged();
}

bool EditorEngineProxy::IsInitialized() const
{
	QMutexLocker lockGuard(&m_nativeEngineMutex);

	return m_editorEngine.IsInitialized();
}

bool EditorEngineProxy::IsRunning() const
{
	QMutexLocker lockGuard(&m_nativeEngineMutex);

	return m_editorEngine.IsRunning();
}

bool EditorEngineProxy::IsSimulating() const
{
	QMutexLocker lockGuard(&m_nativeEngineMutex);

	return m_editorEngine.IsSimulating();
}

// TODO: Better way of achieving this functionality without having to write all of these wrapper classes (?)
eastl::vector<eastl::shared_ptr<MAD::OGameWorld>> EditorEngineProxy::GetGameWorlds() const
{
	QMutexLocker lockGuard(&m_nativeEngineMutex);

	return m_editorEngine.GetWorlds();
}

void EditorEngineProxy::UpdateEntityPosition(eastl::shared_ptr<MAD::AEntity> inEntity, const MAD::Vector3& inNewPosition)
{
	QMutexLocker lockGuard(&m_nativeEngineMutex);

	inEntity->SetWorldTranslation(inNewPosition);
}

void EditorEngineProxy::UpdateEntityRotation(eastl::shared_ptr<MAD::AEntity> inEntity, const MAD::Quaternion& inNewRotation)
{
	QMutexLocker lockGuard(&m_nativeEngineMutex);

	inEntity->SetWorldRotation(inNewRotation);
}

void EditorEngineProxy::UpdateEntityScale(eastl::shared_ptr<MAD::AEntity> inEntity, float inNewScale)
{
	QMutexLocker lockGuard(&m_nativeEngineMutex);

	inEntity->SetWorldScale(inNewScale);
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
		QMutexLocker lockGuard(&m_nativeEngineMutex);

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
