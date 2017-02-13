#pragma once
#include <QFrame>
#include <QMouseEvent>

class EditorSceneViewFrame : public QFrame
{
	Q_OBJECT

public:
	EditorSceneViewFrame(QWidget* parent = Q_NULLPTR);
	~EditorSceneViewFrame();

	virtual void mousePressEvent(QMouseEvent* event) override;
	virtual void keyPressEvent(QKeyEvent* event) override;
	virtual void keyReleaseEvent(QKeyEvent* event) override;
};
