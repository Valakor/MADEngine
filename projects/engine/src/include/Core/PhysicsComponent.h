#pragma once

#include "Core/Component.h"
#include "Core/Entity.h"

namespace MAD
{
	class UPhysicsComponent : public UComponent
	{
		MAD_DECLARE_COMPONENT(UPhysicsComponent, UComponent)
	public:
		virtual void UpdateComponent(float inDeltaTime) override;
	};
}
