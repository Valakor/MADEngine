#include "Core/PointLightComponent.h"
#include "Rendering/Renderer.h"
#include "Core/GameEngine.h"

namespace MAD
{
	CPointLightComponent::CPointLightComponent(OGameWorld* inOwningWorld)
		: Super(inOwningWorld) {}

	void CPointLightComponent::TEMPInitializePointLight(const Vector3& inPosition, const Color& inColor, float inIntensity, float inInnerRadius, float inOuterRadius)
	{
		m_pointLight.m_isLightEnabled = true;
		m_pointLight.m_gpuPointLight.m_lightColor = inColor;
		m_pointLight.m_gpuPointLight.m_lightPosition = inPosition;
		m_pointLight.m_gpuPointLight.m_lightIntensity = inIntensity;
		m_pointLight.m_gpuPointLight.m_lightInnerRadius = inInnerRadius;
		m_pointLight.m_gpuPointLight.m_lightOuterRadius = inOuterRadius;
	}

	void CPointLightComponent::UpdateComponent(float inDeltaTime)
	{
		(void)inDeltaTime;
		// TODO Update transform properly

		if (m_pointLight.m_isLightEnabled)
		{
			URenderer& targetRenderer = gEngine->GetRenderer();
			targetRenderer.QueuePointLight(m_pointLight.m_gpuPointLight);
		}
	}
}