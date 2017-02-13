#pragma once

#include <QApplication>
#include <Core/GameEngine.h>

namespace AM
{
	class EditorApplication : public QApplication
	{
		Q_OBJECT

	public:
		EditorApplication(int& argc, char** argv);
		~EditorApplication();

		void InitApplication();
	private:
		MAD::UGameEngine m_nativeGameEngine;
	};
}
