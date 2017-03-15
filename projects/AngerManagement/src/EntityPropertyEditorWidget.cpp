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

void EntityPropertyEditorWidget::EntitySelected(QTreeWidgetItem* inSelectedEntityItem)
{
	if (!inSelectedEntityItem || inSelectedEntityItem->type() != EntityTreeWidgetItem::EntityWidgetItemType)
	{
		return;
	}

	EntityTreeWidgetItem* selectedTreeItem = static_cast<EntityTreeWidgetItem*>(inSelectedEntityItem);
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

	// Signal that the position value has changed
	emit OnPositionUpdated(newPosition);
}

void EntityPropertyEditorWidget::UpdateRotation()
{
	float xRotRadians = MAD::ConvertToRadians(ui.XRotText->text().toFloat());
	float yRotRadians = MAD::ConvertToRadians(ui.YRotText->text().toFloat());
	float zRotRadians = MAD::ConvertToRadians(ui.ZRotText->text().toFloat());

	MAD::Quaternion newRotation = MAD::Quaternion::CreateFromYawPitchRoll(yRotRadians, xRotRadians, zRotRadians);

	// Signal that the rotation value has changed
	emit OnRotationUpdated(newRotation);
}

void EntityPropertyEditorWidget::UpdateScale()
{
	float newScale = ui.scaleText->text().toFloat();

	// Signal that the scale value has changed
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

	ui.XRotText->setText(QString::number(0));
	ui.YRotText->setText(QString::number(0));
	ui.ZRotText->setText(QString::number(0));
	/*float newPitch = 0.0f;
	float newYaw = 0.0f;
	float newRoll = 0.0f;

	MAD::Quaternion normalizedQuaternion;

	inRotation.Normalize(normalizedQuaternion);

	MAD::GetEulerAngles(normalizedQuaternion, newPitch, newYaw, newRoll);

	ui.XRotText->setText(QString::number(newPitch));
	ui.YRotText->setText(QString::number(newYaw));
	ui.ZRotText->setText(QString::number(newRoll));*/
}

void EntityPropertyEditorWidget::UpdateScaleText(float inScale)
{
	ui.scaleText->setText(QString::number(inScale));
}
