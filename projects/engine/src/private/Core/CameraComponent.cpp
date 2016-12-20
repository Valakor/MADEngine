#include "Core/CameraComponent.h"
#include "Core/Entity.h"
#include "Core/GameEngine.h"
#include "Core/GameWindow.h"
#include "Core/GameWorldLoader.h"
#include "Rendering/Renderer.h"
#include "Misc/Logging.h"
#include "Core/GameInput.h"

namespace MAD
{
	DECLARE_LOG_CATEGORY(LogCameraComponent);

	CCameraComponent::CCameraComponent(OGameWorld* inOwningWorld)
		: Super_t(inOwningWorld)
	{
		m_cameraPosInitial = Vector3::Zero;
		m_cameraRotInitial = Quaternion::Identity;

		m_cameraInstance.m_verticalFOV = ConvertToRadians(60.0f);
		m_cameraInstance.m_nearPlaneDistance = 3.0f;
		m_cameraInstance.m_farPlaneDistance = 10000.0f;
		m_cameraInstance.m_exposure = 1.0f;

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

	void CCameraComponent::UpdateComponent(float)
	{
		// Update the renderer's camera's per camera constant buffer values
		m_cameraInstance.m_transform = GetWorldTransform();
		m_cameraInstance.m_transform.SetScale(1.0f);

		auto& renderer = gEngine->GetRenderer();
		renderer.UpdateCameraConstants(m_cameraInstance);
	}

	void CCameraComponent::Load(const UGameWorldLoader& inLoader)
	{
		auto clientWindow = gEngine->GetWindow().GetClientSize();
		const float aspectRatio = static_cast<float>(clientWindow.x) / clientWindow.y;

		inLoader.GetFloat("fov", m_cameraInstance.m_verticalFOV);
		inLoader.GetFloat("near", m_cameraInstance.m_nearPlaneDistance);
		inLoader.GetFloat("far", m_cameraInstance.m_farPlaneDistance);
		inLoader.GetFloat("exposure", m_cameraInstance.m_exposure);

		inLoader.GetFloat("moveSpeed", m_cameraMoveSpeed);
		inLoader.GetFloat("lookSpeed", m_cameraLookSpeed);

		m_cameraPosInitial = GetWorldTranslation();
		m_cameraRotInitial = GetWorldRotation();

		m_cameraInstance.m_verticalFOV = ConvertToRadians(m_cameraInstance.m_verticalFOV);
		UpdateComponent(0.0f);
	}

	void CCameraComponent::MoveRight(float inVal)
	{
		Vector3 right = Vector3::Transform(Vector3::Right, GetWorldRotation());
		SetWorldTranslation(GetWorldTranslation() + right * inVal * gEngine->GetDeltaTime() * m_cameraMoveSpeed);
	}

	void CCameraComponent::MoveForward(float inVal)
	{
		Vector3 forward = Vector3::Transform(Vector3::Forward, GetWorldRotation());
		SetWorldTranslation(GetWorldTranslation() + forward * inVal * gEngine->GetDeltaTime() * m_cameraMoveSpeed);
	}

	void CCameraComponent::MoveUp(float inVal)
	{
		SetWorldTranslation(GetWorldTranslation() +  Vector3::Up * inVal * gEngine->GetDeltaTime() * m_cameraMoveSpeed);
	}

	void CCameraComponent::LookRight(float inVal)
	{
		if (m_mouseRightClickDown)
		{
			auto rot = Quaternion::CreateFromAxisAngle(Vector3::Up, -inVal * gEngine->GetDeltaTime() * m_cameraLookSpeed);
			SetWorldRotation(GetWorldRotation() * rot);
		}
	}

	void CCameraComponent::LookUp(float inVal)
	{
		if (m_mouseRightClickDown)
		{
			Vector3 right = Vector3::Transform(Vector3::Right, GetWorldRotation());
			auto rot = Quaternion::CreateFromAxisAngle(right, -inVal * gEngine->GetDeltaTime() * m_cameraLookSpeed);
			SetWorldRotation(GetWorldRotation() * rot);
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
		SetWorldTranslation(m_cameraPosInitial);
		SetWorldRotation(m_cameraRotInitial);
	}
}
