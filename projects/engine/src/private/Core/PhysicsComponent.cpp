#include "Core/PhysicsComponent.h"

namespace MAD
{

	UPhysicsComponent::UPhysicsComponent(AEntity& inCompOwner) : Super(inCompOwner) {}

	void UPhysicsComponent::UpdateComponent(float inDeltaTime)
	{
		(void)inDeltaTime;
	}

}