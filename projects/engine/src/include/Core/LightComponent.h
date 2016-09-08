#pragma once

#include "Core/Component.h"

namespace MAD
{
	class CLightComponent : public UComponent
	{
		MAD_DECLARE_PRIORITIZED_COMPONENT(CLightComponent, UComponent, 1)
	public:
		explicit CLightComponent(OGameWorld* inOwningWorld);
		
		virtual void UpdateComponent(float inDeltaTime) override;
	};
}
