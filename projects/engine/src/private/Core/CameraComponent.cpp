#include "Core/CameraComponent.h"
#include "Core/Entity.h"
#include "Core/GameEngine.h"
#include "Core/GameWindow.h"
#include "Rendering/Renderer.h"
#include "Misc/Logging.h"
#include "Core/GameInput.h"

namespace MAD
{
	DECLARE_LOG_CATEGORY(LogCameraComponent);

	CCameraComponent::CCameraComponent(OGameWorld* inOwningWorld)
		: Super(inOwningWorld)
	{
		auto& cameraScheme = *UGameInput::Get().GetControlScheme("CameraDebug");
		cameraScheme.BindAxis<CCameraComponent, &CCameraComponent::MoveForward>("Forward", this);
		cameraScheme.BindAxis<CCameraComponent, &CCameraComponent::MoveRight>("Horizontal", this);
		cameraScheme.BindAxis<CCameraComponent, &CCameraComponent::MoveUp>("Vertical", this);
		cameraScheme.BindAxis<CCameraComponent, &CCameraComponent::LookRight>("LookX", this);
		cameraScheme.BindAxis<CCameraComponent, &CCameraComponent::LookUp>("LookY", this);

		cameraScheme.BindEvent<CCameraComponent, &CCameraComponent::OnMouseRightClickDown>("RightClick", EInputEvent::IE_KeyDown, this);
		cameraScheme.BindEvent<CCameraComponent, &CCameraComponent::OnMouseRightClickUp>("RightClick", EInputEvent::IE_KeyUp, this);
		cameraScheme.BindEvent<CCameraComponent, &CCameraComponent::OnReset>("Reset", EInputEvent::IE_KeyDown, this);

		m_mouseRightClickDown = false;
		m_cameraMoveSpeed = 250.0f;
		m_cameraLookSpeed = 1.0f;
	}

	CCameraComponent::~CCameraComponent()
	{
		// TODO unbind from cameraScheme
	}

	void CCameraComponent::UpdateComponent(float inDeltaTime)
	{
		(void)inDeltaTime;

		Matrix translation = Matrix::CreateTranslation(m_cameraPos);
		Matrix rotation = Matrix::CreateFromQuaternion(m_cameraRot);

		//Matrix rotationX = Matrix::CreateRotationX(ConvertToRadians(m_cameraRot.x));
		//Matrix rotationY = Matrix::CreateRotationY(ConvertToRadians(m_cameraRot.y));

		m_cameraInstance.m_viewMatrix = (rotation * translation).Invert();
		m_cameraInstance.m_viewProjectionMatrix = m_cameraInstance.m_viewMatrix * m_cameraInstance.m_projectionMatrix;

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
		m_cameraInstance.m_projectionMatrix = Matrix::CreatePerspectiveFieldOfView(inFOV, aspectRatio, inNearPlane, inFarPlane);
		m_cameraInstance.m_viewProjectionMatrix = m_cameraInstance.m_viewMatrix * m_cameraInstance.m_projectionMatrix;
	}

	void CCameraComponent::MoveRight(float inVal)
	{
		Vector3 right;
		Vector3::Transform(Vector3::Right, m_cameraRot, right);
		m_cameraPos += right * inVal * gEngine->GetDeltaTime() * m_cameraMoveSpeed;
	}

	void CCameraComponent::MoveForward(float inVal)
	{
		Vector3 forward;
		Vector3::Transform(Vector3::Forward, m_cameraRot, forward);
		m_cameraPos += forward * inVal * gEngine->GetDeltaTime() * m_cameraMoveSpeed;
	}

	void CCameraComponent::MoveUp(float inVal)
	{
		m_cameraPos += Vector3::Up * inVal * gEngine->GetDeltaTime() * m_cameraMoveSpeed;
	}

	void CCameraComponent::LookRight(float inVal)
	{
		if (m_mouseRightClickDown)
		{
			auto rot = Quaternion::CreateFromAxisAngle(Vector3::Up, -inVal * gEngine->GetDeltaTime() * m_cameraLookSpeed);
			m_cameraRot *= rot;
		}
	}

	void CCameraComponent::LookUp(float inVal)
	{
		if (m_mouseRightClickDown)
		{
			Vector3 right;
			Vector3::Transform(Vector3::Right, m_cameraRot, right);
			auto rot = Quaternion::CreateFromAxisAngle(right, -inVal * gEngine->GetDeltaTime() * m_cameraLookSpeed);
			m_cameraRot *= rot;
		}
	}

	void CCameraComponent::OnMouseRightClickDown()
	{
		m_mouseRightClickDown = true;
		UGameInput::Get().SetMouseMode(EMouseMode::MM_Game);
	}

	void CCameraComponent::OnMouseRightClickUp()
	{
		m_mouseRightClickDown = false;
		UGameInput::Get().SetMouseMode(EMouseMode::MM_UI);
	}

	void CCameraComponent::OnReset()
	{
		m_cameraPos = Vector3::Zero;
		m_cameraRot = Quaternion::Identity;
	}
}
