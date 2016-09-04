#include "Core/PhysicsWorld.h"
#include "Core/PhysicsComponent.h"

#include <EASTL/algorithm.h>

namespace MAD
{
	void UPhysicsWorld::RegisterPhysicsBody(eastl::weak_ptr<PhysicsBody> inPhysicsBody)
	{
		if (inPhysicsBody.expired())
		{
			return;
		}

		m_physicsComponents.emplace_back(inPhysicsBody);
	}

	void UPhysicsWorld::UnregisterPhysicsBody(ObjectID inPhysicsBodyID)
	{
		// Removes the target physics body from the list of physics bodies that will be updated
		auto findPhysicsBodyPredicate = [inPhysicsBodyID](PhysicsBodyWeakPtr inCurrentPhysicsBody)
		{
			return inCurrentPhysicsBody.lock()->GetObjectID() == inPhysicsBodyID;
		};

		m_physicsComponents.erase(eastl::remove_if(m_physicsComponents.begin(), m_physicsComponents.end(), findPhysicsBodyPredicate), m_physicsComponents.end());
	}

	void UPhysicsWorld::SimulatePhysics()
	{
		const float fixedTimeStep = 1 / 30.0f;

		auto removeComponentLambda = [](const PhysicsBodyWeakPtr& inPhysicsCompPtr)
		{
			return inPhysicsCompPtr.expired();
		};

		// Remove all the stale physics components
		m_physicsComponents.erase(eastl::remove_if(m_physicsComponents.begin(), m_physicsComponents.end(), removeComponentLambda), m_physicsComponents.end());

		for (auto& currentPhysicsComp : m_physicsComponents)
		{
			currentPhysicsComp.lock()->UpdateComponent(fixedTimeStep);
		}
	}

}