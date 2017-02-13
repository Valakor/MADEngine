#pragma once
#include <QObject>
#include <QWindow>

#include <Editor/EditorEngine.h>

class EditorEngineThread : public QObject
{
	Q_OBJECT
public:
	EditorEngineThread(QObject* parent = Q_NULLPTR);
	~EditorEngineThread();

	bool InitializeEngine(WId inWindowHandleId);
public slots:
	void RunEngine();
	void StopEngine();
private:
	MAD::UEditorEngine m_nativeGameEngine;
};
