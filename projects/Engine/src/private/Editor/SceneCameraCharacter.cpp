#include "Editor/SceneCameraCharacter.h"
#include "Core/CameraComponent.h"

namespace MAD
{
	ASceneCameraCharacter::ASceneCameraCharacter(OGameWorld* inOwningWorld)
		: Super_t(inOwningWorld)
		//, m_lookSpeed(1.0f)
		//, m_moveSpeed(300.0f)
	{
		m_cameraComponent = AddComponent<CCameraComponent>();
		SetRootComponent(m_cameraComponent);
	}
}