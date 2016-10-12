#include "Core/PointLightComponent.h"
#include "Rendering/Renderer.h"
#include "Core/GameEngine.h"

namespace MAD
{
	CPointLightComponent::CPointLightComponent(OGameWorld* inOwningWorld)
		: Super(inOwningWorld) {}

	void CPointLightComponent::TEMPInitializePointLight(const Vector3& inPosition, const Color& inColor, float inIntensity, float inInnerRadius, float inOuterRadius)
	{
		memset(&m_pointLight.m_gpuPointLight, 0, sizeof(m_pointLight.m_gpuPointLight));

		m_pointLight.m_isLightEnabled = true;
		m_pointLight.m_gpuPointLight.m_lightColor = inColor;
		m_pointLight.m_gpuPointLight.m_lightPosition = inPosition;
		m_pointLight.m_gpuPointLight.m_lightIntensity = inIntensity;
		m_pointLight.m_gpuPointLight.m_lightInnerRadius = inInnerRadius;
		m_pointLight.m_gpuPointLight.m_lightOuterRadius = inOuterRadius;

		const float Ic = 1.0f / 256.0f;
		const float Li = inIntensity * eastl::max(eastl::max(inColor.x, inColor.y), inColor.z);
		const float d = inOuterRadius - inInnerRadius;
		
		MAD_ASSERT_DESC(Li > Ic, "Max intensity of ligiht must be greater than minimum intensity cutoff");
		MAD_ASSERT_DESC(Li / Ic > 1.0f, "Li / Ic must be > 1 or denominator will be <= 0");
		MAD_ASSERT_DESC(d > 0.0f, "Max distance from light must be greater than 0");

		const float r = 100.0f * d / (sqrt(Li / Ic) - 1.0f);

		m_pointLight.m_gpuPointLight.m_linearCoefficient = 2.0f / r;
		m_pointLight.m_gpuPointLight.m_quadraticCoefficient = 1.0f / (r * r);
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