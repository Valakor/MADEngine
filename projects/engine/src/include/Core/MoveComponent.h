#pragma once

#include "Core/Component.h"
#include "Core/SimpleMath.h"

namespace MAD
{
	class CMoveComponent : public UComponent
	{
		MAD_DECLARE_PRIORITIZED_COMPONENT(CMoveComponent, UComponent, EPriorityLevelReference::EPriorityLevel_Physics + 10)
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

		struct SSQT
		{
			SSQT() : m_scale(1.0f) {}

			Quaternion m_rotation;
			Vector3 m_position;
			float m_scale;

			uint32_t m_tick;
		};

		UComponent* m_targetComponent; // Which component do you want the MoveComponent to target

		SSQT m_deltaTransform;

		float m_targetScale;
		Vector3 m_targetPosition;
		Quaternion m_targetRotation;
	};
}
