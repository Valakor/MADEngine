#include "Rendering/RenderingCommon.h"

namespace MAD
{
	SGPUPointLight SGPUPointLight::Lerp(const SGPUPointLight& a, const SGPUPointLight& b, float t)
	{
		SGPUPointLight ret;

		Vector3::Lerp(a.m_lightPosition, b.m_lightPosition, t, ret.m_lightPosition);
		Color::Lerp(a.m_lightColor, b.m_lightColor, t, ret.m_lightColor);
		ret.m_lightIntensity = MAD::Lerp(a.m_lightIntensity, b.m_lightIntensity, t);
		ret.m_lightInnerRadius = MAD::Lerp(a.m_lightInnerRadius, b.m_lightInnerRadius, t);
		ret.m_lightOuterRadius = MAD::Lerp(a.m_lightOuterRadius, b.m_lightOuterRadius, t);

		return ret;
	}

	SGPUDirectionalLight SGPUDirectionalLight::Lerp(const SGPUDirectionalLight& a, const SGPUDirectionalLight& b, float t)
	{
		SGPUDirectionalLight ret;

		Vector3::Lerp(a.m_lightDirection, b.m_lightDirection, t, ret.m_lightDirection);
		ret.m_lightDirection.Normalize();
		ret.m_lightIntensity = MAD::Lerp(a.m_lightIntensity, b.m_lightIntensity, t);
		Color::Lerp(a.m_lightColor, b.m_lightColor, t, ret.m_lightColor);

		ret.m_viewProjectionMatrix = a.m_viewProjectionMatrix;

		return ret;
	}

	void SGPUDirectionalLight::CalculateViewProjection(float , float , float , float )
	{
		
	}
}
