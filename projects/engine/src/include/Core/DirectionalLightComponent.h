#pragma once

#include "Core/LightComponent.h"
#include "Rendering/LightInstances.h"

namespace MAD
{
	class CDirectionalLightComponent : public CLightComponent
	{
		MAD_DECLARE_PRIORITIZED_COMPONENT(CDirectionalLightComponent, CLightComponent, EPriorityLevelReference::EPriorityLevel_Physics + 1)
	public:
		explicit CDirectionalLightComponent(OGameWorld* inOwningWorld);

		virtual void Load(const class UGameWorldLoader& inLoader, const class UObjectValue& inPropertyObj) override;
		virtual void UpdateComponent(float inDeltaTime) override;

	private:
		SDirectionalLight m_directionalLight;
	};
}
