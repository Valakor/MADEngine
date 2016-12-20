#include "Core/PhysicsComponent.h"
#include "Core/GameWorld.h"

namespace MAD
{

	CPhysicsComponent::CPhysicsComponent(OGameWorld* inOwningWorld)
		: Super_t(inOwningWorld) {}

	void CPhysicsComponent::UpdateComponent(float inDeltaTime)
	{
		(void)inDeltaTime;
	}

}
