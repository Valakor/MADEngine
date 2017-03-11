#pragma once

#include <mutex>

#include <QObject>
#include <QWindow>
#include <QMutex>
#include <QQueue>

#include <EASTL/vector.h>
#include <EASTl/shared_ptr.h>

#include <Editor/EditorEngine.h>

#include <Core/Entity.h>
#include <Core/SimpleMath.h>

class EditorEngineThread : public QObject
{
	Q_OBJECT
public:
	EditorEngineThread(QObject* parent = Q_NULLPTR);
	~EditorEngineThread();

	bool InitializeEngine(WId inWindowHandleId);
	void OnWindowSizeChanged();

	bool IsInitialized() const;
	bool IsRunning() const;
	bool IsSimulating() const;

	eastl::vector<eastl::shared_ptr<class MAD::OGameWorld>> GetGameWorlds() const;

	template <typename EventType, typename... Args>
	void QueueEngineEvent(Args&&... inConstArgs);
public slots:
	void RunEngine();
	void StopEngine();
private:
	MAD::UEditorEngine m_editorEngine;
	QQueue<class QEngineInterfaceEvent*> m_engineEvents;

	mutable QMutex m_engineEventMutex;
	mutable QMutex m_nativeEngineMutex;
};

template <typename EventType, typename... Args>
void EditorEngineThread::QueueEngineEvent(Args&&... inConstArgs)
{
	QMutexLocker eventLockGuard(&m_engineEventMutex);

	// Perfectly forward the arguments into the event type constructor
	m_engineEvents.push_back(new EventType(std::forward<Args>(inConstArgs)...));
}