#include "Core/Character.h"
#include "Core/MeshComponent.h"
#include "Core/CameraComponent.h"
#include "Core/SimpleMath.h"

namespace MAD
{
	ACharacter::ACharacter(OGameWorld* inOwningWorld)
		: Super(inOwningWorld)
	{
		AddComponent<CCameraComponent>();
		AddComponent<CMeshComponent>();
	}

	void ACharacter::OnBeginPlay()
	{
		using namespace DirectX;

		GetFirstComponentByType<CCameraComponent>().lock()->TEMPInitializeCameraInstance(ConvertToRadians(90.0f), 0.1f, 10000.0f, SimpleMath::Matrix::Identity);
	}
}
