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

class EditorEngineProxy : public QObject
{
	Q_OBJECT
public:
	EditorEngineProxy(QObject* parent = Q_NULLPTR);
	~EditorEngineProxy();

	bool InitializeEngine(WId inWindowHandleId);
	void OnWindowSizeChanged();

	bool IsInitialized() const;
	bool IsRunning() const;
	bool IsSimulating() const;

	eastl::vector<eastl::shared_ptr<class MAD::OGameWorld>> GetGameWorlds() const;

	void QueueEngineEvent(class QEngineInterfaceEvent* inEvent);
public slots:
	void RunEngine();
	void StopEngine();
private:
	MAD::UEditorEngine m_editorEngine;
	QQueue<class QEngineInterfaceEvent*> m_engineEvents;

	mutable QMutex m_engineEventMutex;
	mutable QMutex m_nativeEngineMutex;
};
