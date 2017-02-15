#pragma once
#include <QWidget>
#include "ui_entitypropertyeditorwidget.h"


class EntityPropertyEditorWidget : public QWidget
{
	Q_OBJECT
public:
	EntityPropertyEditorWidget(QWidget* parent = Q_NULLPTR);
	~EntityPropertyEditorWidget();

private slots:
	
	void UpdatePosition();
	void UpdateRotation();
	void UpdateScale();
private:
	Ui::EntityPropertyEditorWidget ui;
};
