#pragma once

#include "Core/Component.h"
#include "Rendering/CameraInstance.h"

// TEMP -------------
#include "Core/SimpleMath.h"

namespace MAD
{
	class CCameraComponent : public UComponent
	{
		MAD_DECLARE_COMPONENT(CCameraComponent, UComponent)
	public:
		explicit CCameraComponent(OGameWorld* inOwningWorld);

		virtual void UpdateComponent(float inDeltaTime) override;
	
		// TODO: Eventually change so that the camera uses it's owners transform to update the view matrix.
		// For now, the camera is stationary so we can just set the camera's view matrix
		void TEMPInitializeCameraInstance(float inFOV, float inNearPlane, float inFarPlane, const DirectX::SimpleMath::Matrix& inViewMatrix);
	private:
		SCameraInstance m_cameraInstance;
	};
}