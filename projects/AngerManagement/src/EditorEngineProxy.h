#pragma once

#include <mutex>

#include <QObject>
#include <QWindow>

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

	eastl::vector<eastl::shared_ptr<class MAD::OGameWorld>> GetGameWorlds() const;

	void UpdateEntityPosition(eastl::shared_ptr<MAD::AEntity> inEntity, const MAD::Vector3& inNewPosition);
public slots:
	void RunEngine();
	void StopEngine();
private:
	MAD::UEditorEngine m_editorEngine;
	mutable std::mutex m_nativeEngineMutex;
};
