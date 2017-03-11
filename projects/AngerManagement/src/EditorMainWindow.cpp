#include "EditorMainWindow.h"
#include "EditorApplication.h"
#include "EditorSceneViewFrame.h"
#include "Engine/EngineCoreEvents.h"

#include <QKeyEvent>
#include <QListWidget>
#include <QKeyEvent>
#include <QCloseEvent>
#include <QResizeEvent>

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
	SetupEditorWidgets();
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

void EditorMainWindow::resizeEvent(QResizeEvent*)
{
	// Make sure initialized properly first (need valid window)
	if (qNativeEngine.IsInitialized())
	{
		qNativeEngine.QueueEngineEvent(new QWindowSizeChangedEvent());
	}
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

void EditorMainWindow::SetupEditorWidgets()
{
	connect(ui.entityTreeWidget, SIGNAL(itemClicked(QTreeWidgetItem*, int)), ui.propertyEditorWidget, SLOT(EntitySelected(QTreeWidgetItem*)));
	connect(ui.propertyEditorWidget, SIGNAL(OnPositionUpdated(const MAD::Vector3&)), ui.entityTreeWidget, SLOT(UpdateEntityPosition(const MAD::Vector3&)));
	connect(ui.propertyEditorWidget, SIGNAL(OnRotationUpdated(const MAD::Quaternion&)), ui.entityTreeWidget, SLOT(UpdateEntityRotation(const MAD::Quaternion&)));
	connect(ui.propertyEditorWidget, SIGNAL(OnScaleUpdated(float)), ui.entityTreeWidget, SLOT(UpdateEntityScale(float)));
}
