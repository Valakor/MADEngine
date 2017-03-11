#include "EngineCoreEvents.h"

#include <Rendering/Renderer.h>

void QWindowSizeChangedEvent::ExecuteInterfaceEvent(MAD::UEditorEngine& inEditorEngine)
{
	inEditorEngine.GetRenderer().OnScreenSizeChanged();
}

QMoveEntityEvent::QMoveEntityEvent(eastl::shared_ptr<MAD::AEntity> inTargetEntity, const MAD::Vector3& inNewPosition)
	: QEntityInterfaceEvent(inTargetEntity)
	, m_targetPosition(inNewPosition)
{
}

void QMoveEntityEvent::ExecuteInterfaceEvent(MAD::UEditorEngine&)
{
	// TOOD Probably better to not use direct shared_ptrs, but works for now, probably transition into handles or something that uniquely identifies entities, but don't 
	// directly maintain ownership (weak_ptrs are not performant :*[)
	m_targetEntity->SetWorldTranslation(m_targetPosition);
}

QRotateEntityEvent::QRotateEntityEvent(eastl::shared_ptr<MAD::AEntity> inTargetEntity, const MAD::Quaternion& inNewRotation)
	: QEntityInterfaceEvent(inTargetEntity)
	, m_targetRotation(inNewRotation)
{
}

void QRotateEntityEvent::ExecuteInterfaceEvent(MAD::UEditorEngine&)
{
	m_targetEntity->SetWorldRotation(m_targetRotation);
}

QScaleEntityEvent::QScaleEntityEvent(eastl::shared_ptr<MAD::AEntity> inTargetEntity, float inNewScale)
	: QEntityInterfaceEvent(inTargetEntity)
	, m_targetScale(inNewScale)
{
}

void QScaleEntityEvent::ExecuteInterfaceEvent(MAD::UEditorEngine&)
{
	m_targetEntity->SetWorldScale(m_targetScale);
}
