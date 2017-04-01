#include "EditorEngineThread.h"
#include "Engine/EngineInterfaceEvent.h"

#include <QCoreApplication>
#include <QMutexLocker>

#include <Core/GameWindow.h>
#include <Rendering/Renderer.h>
#include <Core/GameWorld.h>

EditorEngineThread::EditorEngineThread(QObject* parent) : QObject(parent)
{
}

EditorEngineThread::~EditorEngineThread()
{
}

bool EditorEngineThread::InitializeEngine(WId inWindowHandleId)
{
	return m_editorEngine.Init(eastl::make_shared<MAD::UGameWindow>(reinterpret_cast<HWND>(inWindowHandleId)));
}

void EditorEngineThread::OnWindowSizeChanged()
{
	QMutexLocker lockGuard(&m_nativeEngineMutex);

	m_editorEngine.GetRenderer().OnScreenSizeChanged();
}

bool EditorEngineThread::IsInitialized() const
{
	QMutexLocker lockGuard(&m_nativeEngineMutex);

	return m_editorEngine.IsInitialized();
}

bool EditorEngineThread::IsRunning() const
{
	QMutexLocker lockGuard(&m_nativeEngineMutex);

	return m_editorEngine.IsRunning();
}

bool EditorEngineThread::IsSimulating() const
{
	QMutexLocker lockGuard(&m_nativeEngineMutex);

	return m_editorEngine.IsSimulating();
}

// TODO: Better way of achieving this functionality without having to write all of these wrapper classes (?)
eastl::vector<eastl::shared_ptr<MAD::OGameWorld>> EditorEngineThread::GetGameWorlds() const
{
	QMutexLocker lockGuard(&m_nativeEngineMutex);

	return m_editorEngine.GetWorlds();
}

void EditorEngineThread::RunEngine()
{
	// ==================== Engine Thread Main Loop ===============================
	// The editor engine will need to lock the engine tick mutex each tick so that the editor and engine are synchronized in terms of resource access and modification (i.e. Entities)
	m_editorEngine.ExecuteEngineTests();

	while (m_editorEngine.IsRunning())
	{
		// Process all the engine events before letting the engine execute so that
		// all events are thread safe with the engine thread
		{
			QMutexLocker eventLockGuard(&m_engineEventMutex);

			// TODO Since these events aren't really used with the Qt event system, should they really derive from QEvent?
			while (!m_engineEvents.isEmpty())
			{
				QEngineInterfaceEvent* currentEngineEvent = m_engineEvents.dequeue();
				
				currentEngineEvent->ExecuteInterfaceEvent(m_editorEngine);

				delete currentEngineEvent;
			}
		}

		// Lock guard the engine tick mutex in case the Tick() causes an exception
		{
			QMutexLocker lockGuard(&m_nativeEngineMutex);

			m_editorEngine.Tick();
		}
	}

	m_editorEngine.GetWindow().CaptureCursor(false);

	// After the engine is finished running, we can tell the application to quit
	qApp->quit();
}

void EditorEngineThread::StopEngine()
{
	m_editorEngine.Stop();
}
