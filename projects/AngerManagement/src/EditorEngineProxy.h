#pragma once

#include <mutex>

#include <QObject>
#include <QWindow>

#include <Editor/EditorEngine.h>

class EditorEngineProxy : public QObject
{
	Q_OBJECT
public:
	EditorEngineProxy(QObject* parent = Q_NULLPTR);
	~EditorEngineProxy();

	bool InitializeEngine(WId inWindowHandleId);
public slots:
	void RunEngine();
	void StopEngine();
private:
	MAD::UEditorEngine m_editorEngine;
	std::mutex m_nativeEngineMutex;
};
