#pragma once

#include <QMainWindow>
#include <QFocusEvent>
#include <QKeyEvent>
#include <QCloseEvent>

#include <Core/GameEngine.h>
#include "ui_EditorMainWindow.h"

class EditorMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	EditorMainWindow(QWidget *parent = 0);
	~EditorMainWindow();

	virtual void keyPressEvent(QKeyEvent* event) override;
	virtual void closeEvent(QCloseEvent* event) override;

	WId GetSceneViewWindowHandle() const;

private slots:
	void OnEngineInitialize();
private:
	void SetupEngineInitSlots();
	void SetupEditorWidgets();
private:
	Ui::EditorMainWindow ui;
};
