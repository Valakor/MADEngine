#pragma once

#include "Core/Object.h"

#include <EASTL/vector.h>
#include <EASTL/weak_ptr.h>

namespace MAD
{
	class CPhysicsComponent;

	// The PhysicsWorld is responsible for performing collision detection and collision resolution (where the PhysicsComponent is responsible for simulating the rigid bodies)
	class UPhysicsWorld : public UObject
	{
		MAD_DECLARE_CLASS(UPhysicsWorld, UObject)
	public:
		using PhysicsBody_t = CPhysicsComponent;
		using PhysicsBodyWeakPtr_t = eastl::weak_ptr<PhysicsBody_t>;
		using PhysicsComponentContainer_t = eastl::vector<PhysicsBodyWeakPtr_t>;
	public:
		explicit UPhysicsWorld(OGameWorld* inOwningWorld);

		void SimulatePhysics();
	private:
		PhysicsComponentContainer_t m_physicsComponents;
	};
}
