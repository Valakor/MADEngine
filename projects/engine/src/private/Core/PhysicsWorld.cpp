#include "Core/PhysicsWorld.h"
#include "Core/PhysicsComponent.h"
#include "Misc/Logging.h"

#include <EASTL/algorithm.h>

namespace MAD
{
	DECLARE_LOG_CATEGORY(LogPhysicsWorld);

	void UPhysicsWorld::RegisterPhysicsBody(eastl::weak_ptr<PhysicsBody> inPhysicsBody)
	{
		if (inPhysicsBody.expired())
		{
			return;
		}

		m_physicsComponents.emplace_back(inPhysicsBody);
	}

	void UPhysicsWorld::SimulatePhysics()
	{
		LOG(LogPhysicsWorld, Log, "================SIMULATING PHYSICS================\n");
	}

}