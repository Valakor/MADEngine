#include "Editor/SceneCameraCharacter.h"
#include "Core/CameraComponent.h"

namespace MAD
{
	ASceneCameraCharacter::ASceneCameraCharacter(OGameWorld* inOwningWorld)
		: Super_t(inOwningWorld)
	{
		m_cameraComponent = AddComponent<CCameraComponent>();
		SetRootComponent(m_cameraComponent);
	}
}