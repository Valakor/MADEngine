#pragma once

#include "Core/LightComponent.h"

namespace MAD
{
	class CPointLightComponent : public CLightComponent
	{
		MAD_DECLARE_COMPONENT(CPointLightComponent, CLightComponent)
	public:
		explicit CPointLightComponent(OGameWorld* inOwningWorld);
	};
}