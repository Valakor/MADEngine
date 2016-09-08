#pragma once

#include "Core/LightComponent.h"

namespace MAD
{
	class CDirectionalLightComponent : public CLightComponent
	{
		MAD_DECLARE_COMPONENT(CDirectionalLightComponent, CLightComponent)
	public:
		explicit CDirectionalLightComponent(OGameWorld* inOwningWorld);
	};
}