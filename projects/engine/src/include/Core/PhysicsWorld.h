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
		using PhysicsBody = CPhysicsComponent;
		using PhysicsBodyWeakPtr = eastl::weak_ptr<PhysicsBody>;
		using PhysicsComponentContainer = eastl::vector<PhysicsBodyWeakPtr>;
	public:
		explicit UPhysicsWorld(OGameWorld* inOwningWorld);

		void SimulatePhysics();
	private:
		PhysicsComponentContainer m_physicsComponents;
	};
}