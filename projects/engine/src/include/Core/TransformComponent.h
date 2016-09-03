#pragma once

#include "Core/Component.h"
#include "Core/Entity.h"

namespace MAD
{
	class UTransformComponent : public UComponent
	{
		MAD_DECLARE_PRIORITIZED_COMPONENT(UTransformComponent, UComponent, 10)
	public:
		UTransformComponent(AEntity& inCompOwner);

		virtual void UpdateComponent(float inDeltaTime) override;
	};
}
