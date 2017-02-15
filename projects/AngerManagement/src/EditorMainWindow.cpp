#include "EditorMainWindow.h"
#include "EditorApplication.h"
#include "EditorSceneViewFrame.h"
#include <QKeyEvent>
#include <QListWidget>

// Test includes to make sure that we can include files from other projects
#include <Core/GameInput.h>
#include <Misc/Logging.h>
#include <EASTL/algorithm.h>
#include <yojimbo/yojimbo_allocator.h>
#include <DirectXTK/DDSTextureLoader.h>
#include <rapidjson/document.h>
#include <assimp/scene.h>

EditorMainWindow::EditorMainWindow(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	setFocus();
	setFocusPolicy(Qt::FocusPolicy::ClickFocus);

	SetupEngineInitSlots();
}

EditorMainWindow::~EditorMainWindow()
{
}

void EditorMainWindow::keyPressEvent(QKeyEvent* event)
{
	switch (event->key())
	{
	case Qt::Key_Escape:
		if (hasFocus())
		{
			qEditorApp->StopApplication();
		}
		break;
	default:
		QWidget::keyPressEvent(event);
	}
}

void EditorMainWindow::closeEvent(QCloseEvent* event)
{
	// The user pressed the X button, stop the engine
	qEditorApp->StopApplication();

	event->accept();
}

WId EditorMainWindow::GetSceneViewWindowHandle() const
{
	return ui.sceneWindow->winId();
}

void EditorMainWindow::OnEngineInitialize()
{
	// Perform engine specific initialization here because we know the engine has been initialized to a valid state
	MAD::UGameInput::Get().SetMouseMode(MAD::EMouseMode::MM_Game);
	MAD::UGameInput::Get().OnFocusChanged(false);
}

void EditorMainWindow::SetupEngineInitSlots()
{
	connect(qEditorApp, SIGNAL(EngineInitFinished()), this, SLOT(OnEngineInitialize()));
	connect(qEditorApp, SIGNAL(EngineInitFinished()), ui.entityTreeWidget, SLOT(OnEngineInitialize()));
}
