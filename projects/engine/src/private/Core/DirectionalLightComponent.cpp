#include "Core/DirectionalLightComponent.h"
#include "Core/GameEngine.h"
#include "Rendering/Renderer.h"

namespace MAD
{
	CDirectionalLightComponent::CDirectionalLightComponent(OGameWorld* inOwningWorld)
		: Super(inOwningWorld) {}

	void CDirectionalLightComponent::TEMPInitializeDirectionalLight(const DirectX::SimpleMath::Vector3& inDir, const DirectX::SimpleMath::Color& inColor, float inIntensity)
	{
		m_directionalLight.m_isLightEnabled = true;
		m_directionalLight.m_gpuDirectionalLight.m_lightColor = inColor;
		m_directionalLight.m_gpuDirectionalLight.m_lightDirection = inDir;
		m_directionalLight.m_gpuDirectionalLight.m_lightIntensity = inIntensity;
	}

	void CDirectionalLightComponent::UpdateComponent(float inDeltaTime)
	{
		(void)inDeltaTime;
		// TODO Update transform properly

		if (m_directionalLight.m_isLightEnabled)
		{
			URenderer& targetRenderer = gEngine->GetRenderer();
			targetRenderer.QueueDirectionLight(m_directionalLight.m_gpuDirectionalLight);
		}
	}
}
