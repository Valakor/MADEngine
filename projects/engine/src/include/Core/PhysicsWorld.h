#pragma once

#include "Core/Object.h"

#include <EASTL/vector.h>
#include <EASTL/weak_ptr.h>

namespace MAD
{
	class UPhysicsComponent;

	class UPhysicsWorld : public UObject
	{
		MAD_DECLARE_CLASS(UPhysicsWorld, UObject)
	public:
		using PhysicsBody = UPhysicsComponent;
		using PhysicsBodyWeakPtr = eastl::weak_ptr<PhysicsBody>;
		using PhysicsComponentContainer = eastl::vector<PhysicsBodyWeakPtr>;
	public:
		void RegisterPhysicsBody(eastl::weak_ptr<PhysicsBody> inPhysicsBody);

		virtual void SimulatePhysics();
	private:
		PhysicsComponentContainer m_physicsComponents;
	};
}