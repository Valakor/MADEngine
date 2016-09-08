#pragma once

#include "Core/Component.h"
#include "Core/Entity.h"

namespace MAD
{
	class CPhysicsComponent : public UComponent
	{
		MAD_DECLARE_COMPONENT(CPhysicsComponent, UComponent)
	public:
		explicit CPhysicsComponent(OGameWorld* inOwningWorld);

		virtual void UpdateComponent(float inDeltaTime) override;
	};
}
