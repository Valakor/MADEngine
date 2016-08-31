#pragma once

#include "Core/Component.h"

namespace MAD
{
	class UTransformComponent : public UComponent
	{
		MAD_DECLARE_COMPONENT(UTransformComponent, UComponent)

	public:
		UTransformComponent(AEntity& inCompOwner, TickType inCompTickType);

		virtual void UpdateComponent(float inDeltaTime) override;
	};
}
