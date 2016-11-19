#pragma once

#include "Core/LightComponent.h"
#include "Rendering/LightInstances.h"

namespace MAD
{
	class CPointLightComponent : public CLightComponent
	{
		MAD_DECLARE_PRIORITIZED_COMPONENT(CPointLightComponent, CLightComponent, EPriorityLevelReference::EPriorityLevel_Physics + 1)
	public:
		explicit CPointLightComponent(OGameWorld* inOwningWorld);

		virtual void Load(const UGameWorldLoader& inLoader) override;
		virtual void UpdateComponent(float inDeltaTime) override;

		inline void SetEnabled(bool inEnabled) { m_pointLight.m_isLightEnabled = inEnabled; }

	private:
		SPointLight m_pointLight;
	};
}
