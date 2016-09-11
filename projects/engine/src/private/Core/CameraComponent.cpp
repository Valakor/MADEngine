#include "Core/CameraComponent.h"
#include "Core/Entity.h"
#include "Core/GameEngine.h"
#include "Core/GameWindow.h"
#include "Rendering/Renderer.h"
#include "Misc/Logging.h"

namespace MAD
{
	DECLARE_LOG_CATEGORY(LogCameraComponent);

	CCameraComponent::CCameraComponent(OGameWorld* inOwningWorld)
		: Super(inOwningWorld) {}

	void CCameraComponent::UpdateComponent(float inDeltaTime)
	{
		(void)inDeltaTime;

		LOG(LogCameraComponent, Log, "Updating Mesh Component for %s #%d\n", GetOwner().GetTypeInfo()->GetTypeName(), GetOwner().GetObjectID());

		// Update the renderer's camera's per camera constant buffer values
		auto& renderer = gEngine->GetRenderer();

		renderer.UpdateCameraConstants(m_cameraInstance);
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