#include "Core/PhysicsComponent.h"
#include "Core/GameWorld.h"

namespace MAD
{
	MAD_IMPLEMENT_COMPONENT(UPhysicsComponent)

	UPhysicsComponent::UPhysicsComponent(AEntity& inCompOwner) : Super(inCompOwner) {}

	void UPhysicsComponent::UpdateComponent(float inDeltaTime)
	{
		(void)inDeltaTime;
	}

}