#pragma once
#include <QWidget>
#include <QTreeWidgetItem>
#include "ui_entitypropertyeditorwidget.h"

#include <EASTL/shared_ptr.h>
#include <Core/SimpleMath.h>

class EntityPropertyEditorWidget : public QWidget
{
	Q_OBJECT
public:
	EntityPropertyEditorWidget(QWidget* parent = Q_NULLPTR);
	~EntityPropertyEditorWidget();

signals:
	void OnPositionUpdated(const MAD::Vector3& inNewPosition);
	void OnRotationUpdated(const MAD::Quaternion& inNewRotation);
	void OnScaleUpdated(float inNewScale);
public slots:
	void OnEntitySelected(QTreeWidgetItem* inSelectedEntityItem);
private slots:
	void UpdatePosition();
	void UpdateRotation();
	void UpdateScale();
private:
	void UpdatePositionText(const MAD::Vector3& inPosition);
	void UpdateRotationText(const MAD::Quaternion& inRotation);
	void UpdateScaleText(float inScale);
private:
	Ui::EntityPropertyEditorWidget ui;
};
