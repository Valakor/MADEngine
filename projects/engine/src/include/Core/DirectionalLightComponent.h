#pragma once

#include "Core/LightComponent.h"
#include "Core/SimpleMath.h"
#include "Rendering/LightInstances.h"

namespace MAD
{
	class CDirectionalLightComponent : public CLightComponent
	{
		MAD_DECLARE_PRIORITIZED_COMPONENT(CDirectionalLightComponent, CLightComponent, EPriorityLevelReference::EPriorityLevel_Physics + 1)
	public:
		explicit CDirectionalLightComponent(OGameWorld* inOwningWorld);
		void TEMPInitializeDirectionalLight(const DirectX::SimpleMath::Vector3& inDir, const DirectX::SimpleMath::Color& inColor, float inIntensity);

		virtual void UpdateComponent(float inDeltaTime) override;

	private:
		SDirectionalLight m_directionalLight;
	};
}
