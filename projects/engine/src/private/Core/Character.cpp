#include "Core/Character.h"
#include "Core/MeshComponent.h"
#include "Core/CameraComponent.h"
#include "Core/SimpleMath.h"
#include "Core/DirectionalLightComponent.h"

namespace MAD
{
	ACharacter::ACharacter(OGameWorld* inOwningWorld)
		: Super(inOwningWorld)
	{
		AddComponent<CCameraComponent>();
		AddComponent<CMeshComponent>();
		AddComponent<CMeshComponent>();
		AddComponent<CMeshComponent>();
		AddComponent<CDirectionalLightComponent>();
	}

	void ACharacter::OnBeginPlay()
	{
		using namespace DirectX::SimpleMath;

		GetFirstComponentByType<CCameraComponent>().lock()->TEMPInitializeCameraInstance(ConvertToRadians(60.0f), 0.1f, 10000.0f, Matrix::Identity);

		auto meshComps = GetComponentsByType<CMeshComponent>();
		auto rot = Matrix::Identity;// Matrix::CreateRotationY(ConvertToRadians(45.0f));
		meshComps[0].lock()->TEMPInitializeMeshInstance("engine\\meshes\\nanosuit\\nanosuit.obj", Matrix::CreateRotationY(ConvertToRadians(180.0f)) * Matrix::CreateTranslation(Vector3(0, 0, -5.0f)));
		//meshComps[0].lock()->TEMPInitializeMeshInstance("engine\\meshes\\primitives\\sphere.obj", Matrix::CreateTranslation(Vector3(0, 0, -5.0f)));
		meshComps[1].lock()->TEMPInitializeMeshInstance("engine\\meshes\\primitives\\cube.obj", rot * Matrix::CreateTranslation(Vector3(3.0f, 0, -5.0f)));
		meshComps[2].lock()->TEMPInitializeMeshInstance("engine\\meshes\\primitives\\cube.obj", rot * Matrix::CreateTranslation(Vector3(-3.0f, 0, -5.0f)));

		Vector3 dir = Vector3(0.0f, -0.86602540378f, -0.5f);
		dir.Normalize();
		GetFirstComponentByType<CDirectionalLightComponent>().lock()->TEMPInitializeDirectionalLight(dir, Color(1.0f, 1.0f, 1.0f, 1.0f), 1.0f);
	}
}
