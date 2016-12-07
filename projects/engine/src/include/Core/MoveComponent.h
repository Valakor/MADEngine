#pragma once

#include "Core/Component.h"
#include "Core/SimpleMath.h"

namespace MAD
{
	class CMoveComponent : public UComponent
	{
		MAD_DECLARE_COMPONENT(CMoveComponent, UComponent)
	public:
		explicit CMoveComponent(OGameWorld* inOwningWorld);

		virtual void GetReplicatedProperties(eastl::vector<SObjectReplInfo>& inOutReplInfo) const override;
		virtual void UpdateComponent(float inDeltaTime) override;
		virtual void OnEvent(EEventTypes inEventType, void* inEventData) override;

		void SetTargetComponent(UComponent* inTargetComponent);

		void AddDeltaScale(float inDeltaScale);
		void AddDeltaPosition(const Vector3& inDeltaPosition);
		void AddDeltaRotation(const Quaternion& inDeltaRotation);
	private:
		bool HasNonZeroDelta() const;

		void OnRep_TargetScale();
		void OnRep_TargetPosition();
		void OnRep_TargetRotation();

		UComponent* m_targetComponent; // Which component do you want the MoveComponent to target
		eastl::vector<uint8_t> m_deltaByteBuffer;

		float m_deltaScale;
		Vector3 m_deltaPosition;
		Quaternion m_deltaRotation;

		float m_targetScale;
		Vector3 m_targetPosition;
		Quaternion m_targetRotation;
	};
}
