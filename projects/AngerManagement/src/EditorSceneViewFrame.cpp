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
	case Qt::Key_Escape:
	{
		MAD::UGameInput::Get().OnFocusChanged(false);
		setCursor(Qt::ArrowCursor);
		clearFocus();
		qEditorApp->GetMainWindow()->setFocus();
		break;
	}
	default:
		MAD::UGameInput::Get().OnKeyDown(event->key(), false);
	}
}

void EditorSceneViewFrame::keyReleaseEvent(QKeyEvent* event)
{
	MAD::UGameInput::Get().OnKeyUp(event->key());
}

void EditorSceneViewFrame::focusInEvent(QFocusEvent* event)
{
	if (event->reason() == Qt::FocusReason::MouseFocusReason)
	{
		if (event->gotFocus())
		{
			setCursor(Qt::BlankCursor);
			MAD::UGameInput::Get().OnFocusChanged(true);
		}
		else
		{
			MAD::UGameInput::Get().OnFocusChanged(false);
		}
	}
	//QWidget::focusInEvent(event);
}
