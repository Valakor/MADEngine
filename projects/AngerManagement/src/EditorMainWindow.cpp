#include "EditorMainWindow.h"
#include <QKeyEvent>

// Test includes to make sure that we can include files from other projects
#include <Core/GameInput.h>
#include <Misc/Logging.h>
#include <EASTL/algorithm.h>
#include <yojimbo/yojimbo_allocator.h>
#include <DirectXTK/DDSTextureLoader.h>
#include <rapidjson/document.h>
#include <assimp/scene.h>

namespace AM
{
	EditorMainWindow::EditorMainWindow(QWidget* parent)
		: QMainWindow(parent)
	{
		ui.setupUi(this);
		setFocus();
	}

	EditorMainWindow::~EditorMainWindow()
	{
	}

	void EditorMainWindow::mousePressEvent(QMouseEvent* event)
	{
		MAD::UGameInput::Get().OnKeyDown(event->button(), false);
	}

	void EditorMainWindow::keyPressEvent(QKeyEvent* event)
	{
		if (event->key() == Qt::Key_Escape)
		{
			m_nativeGameEngine.Stop();
			qApp->quit();
		}
		else
		{
			MAD::UGameInput::Get().OnKeyDown(event->key(), false);
		}
	}

	void EditorMainWindow::keyReleaseEvent(QKeyEvent* event)
	{
		MAD::UGameInput::Get().OnKeyUp(event->key());
	}

	void EditorMainWindow::InitEditor()
	{
		// Get the HWND of the panel
		HWND editorWindowHandle = reinterpret_cast<HWND>(ui.sceneWindow->winId());

		if (!m_nativeGameEngine.Init("Anger Management", 1080, 900, editorWindowHandle))
		{
		
		}

		m_nativeGameEngine.Run();
	}
}
