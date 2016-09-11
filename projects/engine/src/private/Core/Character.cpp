#include "Core/Character.h"
#include "Core/TransformComponent.h"
#include "Core/LightComponent.h"
#include "Core/MeshComponent.h"
#include "Core/CameraComponent.h"
#include "Core/TestComponents.h"

namespace MAD
{
	ACharacter::ACharacter(OGameWorld* inOwningWorld)
		: Super(inOwningWorld)
	{
		AddComponent<CCameraComponent>();
		AddComponent<CMeshComponent>();
		AddComponent<CTransformComponent>();
	}

	void ACharacter::OnBeginPlay()
	{

	}
}
