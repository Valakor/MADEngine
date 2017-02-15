#include "EntityPropertyEditorWidget.h"
#include "EditorApplication.h"

#include <Core/SimpleMath.h>

EntityPropertyEditorWidget::EntityPropertyEditorWidget(QWidget* parent) : QWidget(parent)
{
	ui.setupUi(this);

	connect(ui.XPosText, SIGNAL(textChanged(QString)), this, SLOT(UpdatePosition()));
	connect(ui.YPosText, SIGNAL(textChanged(QString)), this, SLOT(UpdatePosition()));
	connect(ui.ZPosText, SIGNAL(textChanged(QString)), this, SLOT(UpdatePosition()));

	connect(ui.XRotText, SIGNAL(textChanged(QString)), this, SLOT(UpdateRotation()));
	connect(ui.YRotText, SIGNAL(textChanged(QString)), this, SLOT(UpdateRotation()));
	connect(ui.ZRotText, SIGNAL(textChanged(QString)), this, SLOT(UpdateRotation()));

	connect(ui.scaleText, SIGNAL(textChanged(QString)), this, SLOT(UpdateScale()));
}

EntityPropertyEditorWidget::~EntityPropertyEditorWidget()
{
	
}

void EntityPropertyEditorWidget::UpdatePosition()
{
	MAD::Vector3 newPosition;

	newPosition.x = ui.XPosText->text().toFloat();
	newPosition.y = ui.YPosText->text().toFloat();
	newPosition.z = ui.ZPosText->text().toFloat();

	// Tell the engine proxy to update the selected entity's position
}

void EntityPropertyEditorWidget::UpdateRotation()
{
	float xRotRadians = MAD::ConvertToRadians(ui.XRotText->text().toFloat());
	float yRotRadians = MAD::ConvertToRadians(ui.YRotText->text().toFloat());
	float zRotRadians = MAD::ConvertToRadians(ui.ZRotText->text().toFloat());

	MAD::Quaternion newRotation = MAD::Quaternion::CreateFromYawPitchRoll(yRotRadians, xRotRadians, zRotRadians);

	newRotation;

	// Tell the engine proxy to update the selected entity's rotation
}

void EntityPropertyEditorWidget::UpdateScale()
{
	float newScale = ui.scaleText->text().toFloat();

	newScale;

	// Tell the engine proxy to update the selected entity's scale
}
