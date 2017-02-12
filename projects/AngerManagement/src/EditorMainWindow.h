#pragma once

#include <QtWidgets/QMainWindow>

#include <Core/GameEngine.h>
#include "ui_EditorWindow.h"

namespace AM
{
	class EditorMainWindow : public QMainWindow
	{
		Q_OBJECT

	public:
		EditorMainWindow(QWidget *parent = 0);
		~EditorMainWindow();
	
		virtual void mousePressEvent(QMouseEvent* event) override;
		virtual void keyPressEvent(QKeyEvent* event) override;
		virtual void keyReleaseEvent(QKeyEvent* event) override;

		void InitEditor();
	private:
		Ui::AngerManagementClass ui;
		MAD::UGameEngine m_nativeGameEngine;
	};
}
