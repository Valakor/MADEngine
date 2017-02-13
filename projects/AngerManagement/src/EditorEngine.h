#pragma once
#include <QObject>
#include <QWindow>

#include <Core/GameEngine.h>

class EditorEngine : public QObject
{
	Q_OBJECT
public:
	EditorEngine(QObject* parent = Q_NULLPTR);
	~EditorEngine();

	bool InitializeEngine(WId inWindowHandleId);
public slots:
	void RunEngine();
	void StopEngine();
private:
	MAD::UGameEngine m_nativeGameEngine;
};
