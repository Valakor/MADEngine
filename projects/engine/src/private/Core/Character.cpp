#include "Core/Character.h"
#include "Core/TransformComponent.h"
#include "Core/LightComponent.h"
#include "Core/MeshComponent.h"
#include "Core/TestComponents.h"

namespace MAD
{
	ACharacter::ACharacter()
	{
		// Within the constructor, you can add components and edit the component's values, but you CANNOT access the owner's owning GameWorldLayer or owning GameWorld
		// Add components to the character
		AddComponent<UTransformComponent>();
		AddComponent<ULightComponent>();
		AddComponent<UTransformComponent>();
		AddComponent<UMeshComponent>();
		AddComponent<UTransformComponent>();
		AddComponent<UTestComponent1>();
		AddComponent<UTestComponent2>();
		AddComponent<UTestComponent3>();
		AddComponent<UTestComponent5>();
		AddComponent<UTestComponent6>();
		AddComponent<UTestComponentA>();
		AddComponent<UTestComponentB>();
		AddComponent<UTestComponentE>();
	}

	void ACharacter::OnBeginPlay()
	{

	}
}