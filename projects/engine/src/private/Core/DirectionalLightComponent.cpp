#include "Core/DirectionalLightComponent.h"
#include "Core/GameEngine.h"
#include "Core/Pipeline/GameWorldLoader.h"
#include "Rendering/Renderer.h"

namespace MAD
{
	CDirectionalLightComponent::CDirectionalLightComponent(OGameWorld* inOwningWorld)
		: Super_t(inOwningWorld)
	{
		m_directionalLight.m_isLightEnabled = false;
		m_directionalLight.m_gpuDirectionalLight.m_lightColor = Color(1.0f, 1.0f, 1.0f, 1.0f);
		m_directionalLight.m_gpuDirectionalLight.m_lightDirection = Vector3::Forward;
		m_directionalLight.m_gpuDirectionalLight.m_lightIntensity = 1.0f;
	}

	void CDirectionalLightComponent::UpdateComponent(float)
	{
		if (m_directionalLight.m_isLightEnabled)
		{
			URenderer& targetRenderer = gEngine->GetRenderer();
			targetRenderer.QueueDirectionalLight(GetObjectID(), m_directionalLight.m_gpuDirectionalLight);
		}
	}

	void CDirectionalLightComponent::Load(const UGameWorldLoader& inLoader)
	{
		inLoader.GetBool("enabled", m_directionalLight.m_isLightEnabled);
		inLoader.GetColor("color", m_directionalLight.m_gpuDirectionalLight.m_lightColor);
		inLoader.GetVector("direction", m_directionalLight.m_gpuDirectionalLight.m_lightDirection);
		inLoader.GetFloat("intensity", m_directionalLight.m_gpuDirectionalLight.m_lightIntensity);

		m_directionalLight.m_gpuDirectionalLight.m_lightDirection.Normalize();
	}
}
