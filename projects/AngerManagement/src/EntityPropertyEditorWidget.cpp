#include "EntityPropertyEditorWidget.h"
#include "EditorApplication.h"
#include "EntityViewerTreeWidget.h"

#include <Core/Entity.h>
#include <Core/SimpleMath.h>

EntityPropertyEditorWidget::EntityPropertyEditorWidget(QWidget* parent) : QWidget(parent)
{
	ui.setupUi(this);

	connect(ui.XPosText, SIGNAL(returnPressed()), this, SLOT(UpdatePosition()));
	connect(ui.YPosText, SIGNAL(returnPressed()), this, SLOT(UpdatePosition()));
	connect(ui.ZPosText, SIGNAL(returnPressed()), this, SLOT(UpdatePosition()));

	connect(ui.XRotText, SIGNAL(returnPressed()), this, SLOT(UpdateRotation()));
	connect(ui.YRotText, SIGNAL(returnPressed()), this, SLOT(UpdateRotation()));
	connect(ui.ZRotText, SIGNAL(returnPressed()), this, SLOT(UpdateRotation()));

	connect(ui.scaleText, SIGNAL(returnPressed()), this, SLOT(UpdateScale()));
}

EntityPropertyEditorWidget::~EntityPropertyEditorWidget()
{
}

void EntityPropertyEditorWidget::OnEntitySelected(QTreeWidgetItem* inSelectedEntityItem)
{
	if (!inSelectedEntityItem)
	{
		return;
	}

	EntityTreeWidgetItem* selectedTreeItem = reinterpret_cast<EntityTreeWidgetItem*>(inSelectedEntityItem);
	eastl::shared_ptr<MAD::AEntity> selectedEntity = selectedTreeItem->GetNativeEntity();

	UpdatePositionText(selectedEntity->GetWorldTranslation());
	UpdateRotationText(selectedEntity->GetWorldRotation());
	UpdateScaleText(selectedEntity->GetWorldScale());
}

void EntityPropertyEditorWidget::UpdatePosition()
{
	MAD::Vector3 newPosition;

	newPosition.x = ui.XPosText->text().toFloat();
	newPosition.y = ui.YPosText->text().toFloat();
	newPosition.z = ui.ZPosText->text().toFloat();

	// Tell the engine proxy to update the selected entity's position
	emit OnPositionUpdated(newPosition);
}

void EntityPropertyEditorWidget::UpdateRotation()
{
	float xRotRadians = MAD::ConvertToRadians(ui.XRotText->text().toFloat());
	float yRotRadians = MAD::ConvertToRadians(ui.YRotText->text().toFloat());
	float zRotRadians = MAD::ConvertToRadians(ui.ZRotText->text().toFloat());

	MAD::Quaternion newRotation = MAD::Quaternion::CreateFromYawPitchRoll(yRotRadians, xRotRadians, zRotRadians);

	newRotation;

	emit OnRotationUpdated(MAD::Quaternion::Identity);
}

void EntityPropertyEditorWidget::UpdateScale()
{
	float newScale = ui.scaleText->text().toFloat();

	emit OnScaleUpdated(newScale);
}

void EntityPropertyEditorWidget::UpdatePositionText(const MAD::Vector3& inPosition)
{
	ui.XPosText->setText(QString::number(inPosition.x));
	ui.YPosText->setText(QString::number(inPosition.y));
	ui.ZPosText->setText(QString::number(inPosition.z));
}

void EntityPropertyEditorWidget::UpdateRotationText(const MAD::Quaternion& inRotation)
{
	UNREFERENCED_PARAMETER(inRotation);
}

void EntityPropertyEditorWidget::UpdateScaleText(float inScale)
{
	ui.scaleText->setText(QString::number(inScale));
}
