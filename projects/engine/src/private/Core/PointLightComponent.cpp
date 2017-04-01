#include "Core/PointLightComponent.h"
#include "Rendering/Renderer.h"
#include "Core/GameEngine.h"
#include "Core/Pipeline/GameWorldLoader.h"

namespace MAD
{
	CPointLightComponent::CPointLightComponent(OGameWorld* inOwningWorld)
		: Super_t(inOwningWorld)
	{
		m_pointLight.m_isLightEnabled = false;
		m_pointLight.m_gpuPointLight.m_lightColor = Color(1.0f, 1.0f, 1.0f, 1.0f);
		m_pointLight.m_gpuPointLight.m_lightPosition = Vector3::Zero;
		m_pointLight.m_gpuPointLight.m_lightIntensity = 2.0f;
		m_pointLight.m_gpuPointLight.m_lightInnerRadius = 50;
		m_pointLight.m_gpuPointLight.m_lightOuterRadius = 300;
	}

	void CPointLightComponent::UpdateComponent(float)
	{
		if (m_pointLight.m_isLightEnabled)
		{
			m_pointLight.m_gpuPointLight.m_lightPosition = GetWorldTranslation();

			URenderer& targetRenderer = gEngine->GetRenderer();
			targetRenderer.QueuePointLight(GetObjectID(), m_pointLight.m_gpuPointLight);
		}
	}

	void CPointLightComponent::Load(const UGameWorldLoader& inLoader, const UObjectValue& inPropertyObj)
	{
		UNREFERENCED_PARAMETER(inLoader);

		inPropertyObj.GetProperty("enabled", m_pointLight.m_isLightEnabled);
		inPropertyObj.GetProperty("color", m_pointLight.m_gpuPointLight.m_lightColor);
		inPropertyObj.GetProperty("intensity", m_pointLight.m_gpuPointLight.m_lightIntensity);
		inPropertyObj.GetProperty("innerRadius", m_pointLight.m_gpuPointLight.m_lightInnerRadius);
		inPropertyObj.GetProperty("outerRadius", m_pointLight.m_gpuPointLight.m_lightOuterRadius);
	}

}
