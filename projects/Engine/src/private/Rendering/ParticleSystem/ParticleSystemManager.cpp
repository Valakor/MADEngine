#include "Rendering/ParticleSystem/ParticleSystemManager.h"
#include "Rendering/ParticleSystem/ParticleSystem.h"
#include "Rendering/RenderContext.h"
#include "Rendering/Renderer.h"
#include "Rendering/GraphicsDriver.h"

#include "Misc/utf8conv.h"

namespace MAD
{
	UParticleSystemManager::UParticleSystemManager() : m_firstInactiveParticleSystem(0)
	{
	}

	void UParticleSystemManager::UpdateParticleSystems(float inDeltaTime)
	{
		for (size_t i = 0; i < m_firstInactiveParticleSystem; ++i)
		{
			URenderContext::Get().GetGraphicsDriver().StartEventGroup(utf8util::UTF16FromUTF8(m_particleSystemPool[i].GetSystemName().c_str()));

			m_particleSystemPool[i].TickSystem(inDeltaTime);

			URenderContext::Get().GetGraphicsDriver().EndEventGroup();
		}
	}

	UParticleSystem* UParticleSystemManager::ActivateParticleSystem(const SParticleSystemSpawnParams& inSystemParams, const eastl::vector<SParticleEmitterSpawnParams>& inEmitterParams)
	{
		if (m_firstInactiveParticleSystem == UParticleSystemManager::s_maxParticleSystems)
		{
			MAD_ASSERT_DESC(false, "Max active particle systems has been met!");
			return false;
		}

		UParticleSystem* activatedSystem = &m_particleSystemPool[m_firstInactiveParticleSystem];

		activatedSystem->Initialize(inSystemParams, inEmitterParams);

		++m_firstInactiveParticleSystem;

		return activatedSystem;
	}

	bool UParticleSystemManager::DeactivateParticleSystem(const UParticleSystem* inTargetParticleSystem)
	{
		if (m_firstInactiveParticleSystem == 0 || !inTargetParticleSystem)
		{
			// Can't deactivate particle system if none exists
			return false;
		}

		const size_t numActiveParticleSystems = m_firstInactiveParticleSystem;

		for (size_t i = 0; i < numActiveParticleSystems; ++i)
		{
			if (inTargetParticleSystem == &m_particleSystemPool[i])
			{
				eastl::swap(m_particleSystemPool[i], m_particleSystemPool[m_firstInactiveParticleSystem - 1]);

				--m_firstInactiveParticleSystem;

				return true;
			}
		}

		return false;
	}
}