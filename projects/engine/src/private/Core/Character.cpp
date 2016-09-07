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
	}

	void ACharacter::OnBeginPlay()
	{

	}
}
