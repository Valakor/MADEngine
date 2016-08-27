#pragma once

#include "Core\Component.h"
#include "Core\GameWorld.h"

namespace MAD
{
	class UTransformComponent : public UComponent
	{
		MAD_DECLARE_COMPONENT(UTransformComponent)
	public:
		UTransformComponent(AEntity* inCompOwner, TickType inCompTickType);

		virtual void UpdateComponent(float inDeltaTime) override;
	};
}
