#include "Core/MoveComponent.h"
#include "Misc/Logging.h"

namespace MAD
{
	DECLARE_LOG_CATEGORY(LogMoveComponent);

	CMoveComponent::CMoveComponent(OGameWorld* inOwningWorld)
		: Super(inOwningWorld)
		, m_targetComponent(nullptr)
		, m_targetScale(0.0f)
	{
	}

	void CMoveComponent::GetReplicatedProperties(eastl::vector<SObjectReplInfo>& inOutReplInfo) const
	{
		Super::GetReplicatedProperties(inOutReplInfo);

		MAD_ADD_REPLICATION_PROPERTY_CALLBACK(inOutReplInfo, EReplicationType::Always, CMoveComponent, m_targetScale,    OnRep_TargetScale);
		MAD_ADD_REPLICATION_PROPERTY_CALLBACK(inOutReplInfo, EReplicationType::Always, CMoveComponent, m_targetRotation, OnRep_TargetRotation);
		MAD_ADD_REPLICATION_PROPERTY_CALLBACK(inOutReplInfo, EReplicationType::Always, CMoveComponent, m_targetPosition, OnRep_TargetPosition);
	}

	void CMoveComponent::UpdateComponent(float)
	{
		if (GetNetMode() == ENetMode::DedicatedServer)
		{
			return;
		}

		if (!m_targetComponent)
		{
			return;
		}

		if (!HasNonZeroDelta())
		{
			return;
		}

		m_deltaTransform.m_tick = gEngine->GetGameTick();

		// Send network event message to the server requesting a move based off of the deltas
		gEngine->GetNetworkManager().SendNetworkEvent(EEventTarget::Server, MOVE_ENTITY, GetOwningEntity(), &m_deltaTransform, sizeof(m_deltaTransform));
	
		// Reset the deltas
		m_deltaTransform.m_scale = 0.0f;
		m_deltaTransform.m_rotation = Quaternion::Identity;
		m_deltaTransform.m_position = Vector3::Zero;
	}

	void CMoveComponent::OnEvent(EEventTypes inEventType, void* inEventData)
	{
		if (inEventType == MOVE_ENTITY && GetNetMode() != ENetMode::Client)
		{
			// [-----------------------Server Only-----------------------------]

			// Data contains the delta scale, position, and rotation (in that order)
			const SSQT* deltaTransform = reinterpret_cast<const SSQT*>(inEventData);

			// Potentially validate the delta values here
			
			// Apply the deltas to the server's representation of the owner
			m_targetComponent->SetWorldScale(m_targetComponent->GetWorldScale() + deltaTransform->m_scale);
			m_targetComponent->SetWorldRotation(m_targetComponent->GetWorldRotation() * deltaTransform->m_rotation);
			m_targetComponent->SetWorldTranslation(m_targetComponent->GetWorldTranslation() + deltaTransform->m_position);

			// Changing the target transform state will cause replication to the clients
			m_targetScale += deltaTransform->m_scale;
			m_targetRotation *= deltaTransform->m_rotation;
			m_targetPosition += deltaTransform->m_position;
		}
	}

	void CMoveComponent::SetTargetComponent(UComponent* inTargetComponent)
	{
		if (!inTargetComponent)
		{
			LOG(LogMoveComponent, Warning, "Trying to target a null component!\n");
			return;
		}

		m_targetComponent = inTargetComponent;

		m_targetScale = m_targetComponent->GetWorldScale();
		m_targetRotation = m_targetComponent->GetWorldRotation();
		m_targetPosition = m_targetComponent->GetWorldTranslation();
	}

	void CMoveComponent::AddDeltaScale(float inDeltaScale)
	{
		m_deltaTransform.m_scale += inDeltaScale;
	}

	void CMoveComponent::AddDeltaPosition(const Vector3& inDeltaPosition)
	{
		m_deltaTransform.m_position += inDeltaPosition;
	}

	void CMoveComponent::AddDeltaRotation(const Quaternion& inDeltaRotation)
	{
		m_deltaTransform.m_rotation *= inDeltaRotation;
	}

	bool CMoveComponent::HasNonZeroDelta() const
	{
		return m_deltaTransform.m_scale != 0.0f || m_deltaTransform.m_rotation != Quaternion::Identity || m_deltaTransform.m_position != Vector3::Zero;
	}

	void CMoveComponent::OnRep_TargetScale()
	{
		// Update the target component's scale
		m_targetComponent->SetWorldScale(m_targetScale);
	}

	void CMoveComponent::OnRep_TargetPosition()
	{
		// Update the target component's position
		m_targetComponent->SetWorldTranslation(m_targetPosition);
	}

	void CMoveComponent::OnRep_TargetRotation()
	{
		// Update the target component's rotation
		m_targetComponent->SetWorldRotation(m_targetRotation);
	}
}