#pragma once

#include <QMainWindow>
#include <QFocusEvent>
#include <QKeyEvent>

#include <Core/GameEngine.h>
#include "ui_EditorMainWindow.h"

class EditorMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	EditorMainWindow(QWidget *parent = 0);
	~EditorMainWindow();

	virtual void keyPressEvent(QKeyEvent* event) override;

	WId GetSceneViewWindowHandle() const;
	private slots:
	void OnEngineInitialize();
private:
	Ui::EditorMainWindow ui;
};
