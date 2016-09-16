#pragma once

#include "Core/Component.h"
#include "Rendering/CameraInstance.h"

// TEMP -------------
#include "Core/SimpleMath.h"

namespace MAD
{
	class CCameraComponent : public UComponent
	{
		MAD_DECLARE_PRIORITIZED_COMPONENT(CCameraComponent, UComponent, EPriorityLevelReference::EPriorityLevel_Physics + 1)
	public:
		explicit CCameraComponent(OGameWorld* inOwningWorld);
		virtual ~CCameraComponent();

		virtual void UpdateComponent(float inDeltaTime) override;
	
		// TODO: Eventually change so that the camera uses it's owners transform to update the view matrix.
		// For now, the camera is stationary so we can just set the camera's view matrix
		void TEMPInitializeCameraInstance(float inFOV, float inNearPlane, float inFarPlane, const DirectX::SimpleMath::Matrix& inViewMatrix);
	private:
		void MoveRight(float inVal);
		void MoveForward(float inVal);
		void MoveUp(float inVal);

		void LookRight(float inVal);
		void LookUp(float inVal);

		void OnMouseRightClickDown();
		void OnMouseRightClickUp();

		void OnReset();

		SCameraInstance m_cameraInstance;
		Vector3 m_cameraPos;
		Quaternion m_cameraRot; // pitch, yaw, roll

		bool m_mouseRightClickDown;
	};
}