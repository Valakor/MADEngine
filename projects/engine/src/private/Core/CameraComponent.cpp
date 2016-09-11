#include "Core/CameraComponent.h"
#include "Core/GameEngine.h"
#include "Core/GameWindow.h"

namespace MAD
{
	CCameraComponent::CCameraComponent(OGameWorld* inOwningWorld)
		: Super(inOwningWorld) {}

	void CCameraComponent::UpdateComponent(float inDeltaTime)
	{
		(void)inDeltaTime;

		// Update the renderer's camera's per camera constant buffer values
	}

	void CCameraComponent::TEMPInitializeCameraInstance(float inFOV, float inNearPlane, float inFarPlane, const DirectX::SimpleMath::Matrix& inViewMatrix)
	{
		auto clientWindow = gEngine->GetWindow().GetClientSize();
		const float aspectRatio = static_cast<float>(clientWindow.x) / clientWindow.y;

		m_cameraInstance.m_verticalFOV = inFOV;
		m_cameraInstance.m_nearPlaneDistance = inNearPlane;
		m_cameraInstance.m_farPlaneDistance = inFarPlane;
		m_cameraInstance.m_viewMatrix = inViewMatrix;
		m_cameraInstance.m_projectionMatrix = DirectX::SimpleMath::Matrix::CreatePerspectiveFieldOfView(inFOV, aspectRatio, inNearPlane, inFarPlane);
		m_cameraInstance.m_viewProjectionMatrix = m_cameraInstance.m_viewMatrix * m_cameraInstance.m_projectionMatrix;
	}

}