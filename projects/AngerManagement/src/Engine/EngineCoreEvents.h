#pragma once
#include <Engine/EngineInterfaceEvent.h>

#include <Core/Entity.h>
#include <eastl/shared_ptr.h>

// TODO Move this out to separate file for window events?
class QWindowSizeChangedEvent : public QEngineInterfaceEvent
{
public:
	QWindowSizeChangedEvent() {}

	virtual void ExecuteInterfaceEvent(MAD::UEditorEngine& inEditorEngine) override;
};

class QEntityInterfaceEvent : public QEngineInterfaceEvent
{
public:
	QEntityInterfaceEvent(eastl::shared_ptr<MAD::AEntity> inTargetEntity) : m_targetEntity(inTargetEntity) {}

protected:
	eastl::shared_ptr<MAD::AEntity> m_targetEntity;
};

class QMoveEntityEvent : public QEntityInterfaceEvent
{
public:
	QMoveEntityEvent(eastl::shared_ptr<MAD::AEntity> inTargetEntity, const MAD::Vector3& inNewPosition);
	
	virtual void ExecuteInterfaceEvent(MAD::UEditorEngine& inEditorEngine) override;
private:
	const MAD::Vector3 m_targetPosition;
};

class QRotateEntityEvent : public QEntityInterfaceEvent
{
public:
	QRotateEntityEvent(eastl::shared_ptr<MAD::AEntity> inTargetEntity, const MAD::Quaternion& inNewRotation);

	virtual void ExecuteInterfaceEvent(MAD::UEditorEngine& inEditorEngine) override;
private:
	const MAD::Quaternion m_targetRotation;
};

class QScaleEntityEvent : public QEntityInterfaceEvent
{
public:
	QScaleEntityEvent(eastl::shared_ptr<MAD::AEntity> inTargetEntity, float inNewScale);

	virtual void ExecuteInterfaceEvent(MAD::UEditorEngine& inEditorEngine) override;
private:
	const float m_targetScale;
};