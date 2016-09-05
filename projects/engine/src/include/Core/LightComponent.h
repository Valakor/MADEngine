#pragma once

#include "Core/Component.h"

namespace MAD
{
	class ULightComponent : public UComponent
	{
		MAD_DECLARE_PRIORITIZED_COMPONENT(ULightComponent, UComponent, 1)
	public:
		virtual void UpdateComponent(float inDeltaTime) override;
	};
}