#pragma once

#include <QMainWindow>

#include <Core/GameEngine.h>
#include "ui_EditorMainWindow.h"

class EditorMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	EditorMainWindow(QWidget *parent = 0);
	~EditorMainWindow();

	virtual void keyPressEvent(class QKeyEvent* event) override;
	virtual void closeEvent(class QCloseEvent* event) override;
	virtual void resizeEvent(class QResizeEvent* event) override;

	WId GetSceneViewWindowHandle() const;

private slots:
	void OnEngineInitialize();
private:
	void SetupEngineInitSlots();
	void SetupEditorWidgets();
private:
	Ui::EditorMainWindow ui;
};
