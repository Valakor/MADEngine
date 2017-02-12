#ifndef ANGERMANAGEMENT_H
#define ANGERMANAGEMENT_H

#include <QtWidgets/QMainWindow>

#include <Core/GameEngine.h>
#include "ui_MainEditorWindow.h"

class AngerManagementWindow : public QMainWindow
{
	Q_OBJECT

public:
	AngerManagementWindow(QWidget *parent = 0);
	~AngerManagementWindow();
	
	virtual void mousePressEvent(QMouseEvent* event) override;
	virtual void keyPressEvent(QKeyEvent* event) override;
	virtual void keyReleaseEvent(QKeyEvent* event) override;

	void InitEditor();
private:
	Ui::AngerManagementClass ui;
	MAD::UGameEngine m_nativeGameEngine;
};

#endif // ANGERMANAGEMENT_H
