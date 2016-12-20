#include "Core/PhysicsWorld.h"
#include "Core/PhysicsComponent.h"
#include "Misc/Logging.h"

#include <EASTL/algorithm.h>

namespace MAD
{
	DECLARE_LOG_CATEGORY(LogPhysicsWorld);

	UPhysicsWorld::UPhysicsWorld(OGameWorld* inOwningWorld)
		: Super_t(inOwningWorld) {}

	void UPhysicsWorld::SimulatePhysics()
	{
		//LOG(LogPhysicsWorld, Log, "================SIMULATING PHYSICS================\n");
	}

}
