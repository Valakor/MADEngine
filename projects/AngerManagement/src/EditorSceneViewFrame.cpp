#include "EditorSceneViewFrame.h"
#include "EditorMainWindow.h"
#include "EditorApplication.h"

#include <Core/GameInput.h>

EditorSceneViewFrame::EditorSceneViewFrame(QWidget* parent /*= Q_NULLPTR*/) : QFrame(parent)
{
	setFocusPolicy(Qt::FocusPolicy::ClickFocus);
}

EditorSceneViewFrame::~EditorSceneViewFrame()
{
}

void EditorSceneViewFrame::mousePressEvent(QMouseEvent* event)
{
	MAD::UGameInput::Get().OnKeyDown(event->button(), false);
}

void EditorSceneViewFrame::keyPressEvent(QKeyEvent* event)
{
	switch (event->key())
	{
	case Qt::Key_M:
	{
		MAD::UGameInput::Get().OnKeyDown(event->key(), false);
		clearFocus();
	}
	default:
		QWidget::keyPressEvent(event);
	}
}

void EditorSceneViewFrame::keyReleaseEvent(QKeyEvent* event)
{
	MAD::UGameInput::Get().OnKeyUp(event->key());
}
