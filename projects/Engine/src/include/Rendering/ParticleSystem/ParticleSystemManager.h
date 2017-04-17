#pragma once

#include "Rendering/ParticleSystem/ParticleSystem.h"
#include "Misc/Assert.h"

#include <EASTL/array.h>

namespace MAD
{
	// Responsible for managing alive particle systems and keeps ownership of the particle systems themselves
	class UParticleSystemManager
	{
	public:
		static const size_t s_maxParticleSystems = 1024;
	public:
		UParticleSystemManager();

		void OnScreenSizeChanged();

		void UpdateParticleSystems(float inDeltaTime);
		UParticleSystem* ActivateParticleSystem(const SParticleSystemSpawnParams& inSpawnParams, const eastl::vector<SParticleEmitterSpawnParams>& inEmitterParams);
		bool DeactivateParticleSystem(const UParticleSystem* inTargetParticleSystem);
	private:
		eastl::array<UParticleSystem, s_maxParticleSystems> m_particleSystemPool;
		size_t m_firstInactiveParticleSystem;
	};
}