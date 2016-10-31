#pragma once

#include "Core/LightComponent.h"
#include "Core/SimpleMath.h"
#include "Rendering/LightInstances.h"

namespace MAD
{
	class CPointLightComponent : public CLightComponent
	{
		MAD_DECLARE_PRIORITIZED_COMPONENT(CPointLightComponent, CLightComponent, EPriorityLevelReference::EPriorityLevel_Physics + 1)
	public:
		explicit CPointLightComponent(OGameWorld* inOwningWorld);
		void TEMPInitializePointLight(const Vector3& inPosition, const Color& inColor, float inIntensity, float inInnerRadius, float inOuterRadius);

		virtual void UpdateComponent(float inDeltaTime) override;

	private:
		SPointLight m_pointLight;
	};
}