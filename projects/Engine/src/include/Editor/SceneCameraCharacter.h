#pragma once

#include "Core/Entity.h"

namespace MAD
{
	class ASceneCameraCharacter : public AEntity
	{
		MAD_DECLARE_ACTOR(ASceneCameraCharacter, AEntity)
	public:
		explicit ASceneCameraCharacter(OGameWorld* inOwningWorld);

		/*virtual void OnBeginPlay() override;
	private:
		void Move(const Vector3& inDelta);
		void Look(const Quaternion& inDelta);
		void MoveRight(float inVal);
		void MoveForward(float inVal);
		void MoveUp(float inVal);
		void LookRight(float inVal);
		void LookUp(float inVal);*/
	private:
		eastl::shared_ptr<class CCameraComponent> m_cameraComponent;

		//float m_lookSpeed;
		//float m_moveSpeed;
	};
}