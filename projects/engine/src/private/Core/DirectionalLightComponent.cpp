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

	void CDirectionalLightComponent::Load(const class UGameWorldLoader& inLoader, const UObjectValue& inPropertyObj)
	{
		UNREFERENCED_PARAMETER(inLoader);

		inPropertyObj.GetProperty("enabled", m_directionalLight.m_isLightEnabled);
		inPropertyObj.GetProperty("color", m_directionalLight.m_gpuDirectionalLight.m_lightColor);
		inPropertyObj.GetProperty("direction", m_directionalLight.m_gpuDirectionalLight.m_lightDirection);
		inPropertyObj.GetProperty("intensity", m_directionalLight.m_gpuDirectionalLight.m_lightIntensity);

		m_directionalLight.m_gpuDirectionalLight.m_lightDirection.Normalize();
	}
}
