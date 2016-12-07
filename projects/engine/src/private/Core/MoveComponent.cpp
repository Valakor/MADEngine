#include "Core/MoveComponent.h"
#include "Misc/Logging.h"

namespace MAD
{
	DECLARE_LOG_CATEGORY(LogMoveComponent);

	CMoveComponent::CMoveComponent(OGameWorld* inOwningWorld)
		: Super(inOwningWorld)
		, m_targetComponent(nullptr)
		, m_deltaScale(0.0f)
		, m_targetScale(0.0f)
	{
		m_deltaByteBuffer.resize(sizeof(m_deltaScale) + sizeof(m_deltaRotation) + sizeof(m_deltaPosition));
	}

	void CMoveComponent::GetReplicatedProperties(eastl::vector<SObjectReplInfo>& inOutReplInfo) const
	{
		Super::GetReplicatedProperties(inOutReplInfo);

		MAD_ADD_REPLICATION_PROPERTY_CALLBACK(inOutReplInfo, EReplicationType::Always, CMoveComponent, m_targetScale, OnRep_TargetScale);
		MAD_ADD_REPLICATION_PROPERTY_CALLBACK(inOutReplInfo, EReplicationType::Always, CMoveComponent, m_targetRotation, OnRep_TargetRotation);
		MAD_ADD_REPLICATION_PROPERTY_CALLBACK(inOutReplInfo, EReplicationType::Always, CMoveComponent, m_targetPosition, OnRep_TargetPosition);
	}

	void CMoveComponent::UpdateComponent(float inDeltaTime)
	{
		(void)inDeltaTime;
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

		// Send network event message to the server requesting a move based off of the deltas
		const size_t deltaDataSize = sizeof(m_deltaScale) + sizeof(m_deltaRotation) + sizeof(m_deltaPosition);
		const uint8_t* deltaDataStart = reinterpret_cast<const uint8_t*>(&m_deltaScale);

		memcpy(m_deltaByteBuffer.data(), deltaDataStart, deltaDataSize);
		
		gEngine->GetNetworkManager().SendNetworkEvent(EEventTarget::Server, MOVE_ENTITY, GetOwningEntity(), m_deltaByteBuffer.data(), m_deltaByteBuffer.size());
	
		// Reset the deltas
		m_deltaScale = 0.0f;
		m_deltaRotation = Quaternion::Identity;
		m_deltaPosition = Vector3::Zero;
	}

	void CMoveComponent::OnEvent(EEventTypes inEventType, void* inEventData)
	{
		const uint8_t* byteDataBuffer = reinterpret_cast<const uint8_t*>(inEventData);

		if (inEventType == MOVE_ENTITY && GetNetMode() != ENetMode::Client)
		{
			// [-----------------------Server Only-----------------------------]

			// Data contains the delta scale, position, and rotation (in that order)
			float deltaScale = *reinterpret_cast<const float*>(byteDataBuffer);
			
			byteDataBuffer += sizeof(deltaScale);

			Vector3 deltaPosition = *reinterpret_cast<const Vector3*>(byteDataBuffer);

			byteDataBuffer += sizeof(deltaPosition);

			Quaternion deltaRotation = *reinterpret_cast<const Quaternion*>(byteDataBuffer);

			// Potentially validate the delta values here
			
			// Apply the deltas to the server's representation of the owner
			m_targetComponent->SetWorldScale(m_targetComponent->GetWorldScale() + deltaScale);
			m_targetComponent->SetWorldRotation(Quaternion::Concatenate(m_targetComponent->GetWorldRotation(), deltaRotation));
			m_targetComponent->SetWorldTranslation(m_targetComponent->GetWorldTranslation() + deltaPosition);

			// Changing the target transform state will cause replication to the clients
			m_targetScale += deltaScale;
			m_targetRotation = Quaternion::Concatenate(m_targetRotation, deltaRotation);
			m_targetPosition += deltaPosition;
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
		m_deltaScale += inDeltaScale;
	}

	void CMoveComponent::AddDeltaPosition(const Vector3& inDeltaPosition)
	{
		m_deltaPosition += inDeltaPosition;
	}

	void CMoveComponent::AddDeltaRotation(const Quaternion& inDeltaRotation)
	{
		m_deltaRotation = Quaternion::Concatenate(m_deltaRotation, inDeltaRotation);
	}

	bool CMoveComponent::HasNonZeroDelta() const
	{
		return m_deltaScale != 0.0f || m_deltaRotation != Quaternion::Identity || m_deltaPosition != Vector3::Zero;
	}

	void CMoveComponent::OnRep_TargetScale()
	{
		// Update the target component's scale
		LOG(LogMoveComponent, Warning, "Replicating target scale successful!\n");
		m_targetComponent->SetWorldScale(m_targetScale);
	}

	void CMoveComponent::OnRep_TargetPosition()
	{
		// Update the target component's position
		LOG(LogMoveComponent, Warning, "Replicating target position successful!\n");
		m_targetComponent->SetWorldTranslation(m_targetPosition);
	}

	void CMoveComponent::OnRep_TargetRotation()
	{
		// Update the target component's rotation
		LOG(LogMoveComponent, Warning, "Replicating target rotation successful!\n");
		m_targetComponent->SetWorldRotation(m_targetRotation);
	}
}