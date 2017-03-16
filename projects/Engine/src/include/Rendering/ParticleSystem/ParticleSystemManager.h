#pragma once

#include "Rendering/ParticleSystem/ParticleSystem.h"

#include <EASTL/array.h>

namespace MAD
{
	// Responsible for managing alive particle systems and keeps ownership of the particle systems themselves
	template <size_t MaxParticleSystems = 1024>
	class UParticleSystemManager
	{
	public:
		UParticleSystemManager();

		void UpdateParticleSystems(float inDeltaTime);

		bool ActivateParticleSystem(class UParticleSystem& outParticleSystem);
		void DeactivateParticleSystem(const UParticleSystem& inTargetParticleSystem);
	private:
		eastl::array<UParticleSystem, MaxParticleSystems> m_particleSystemPool;
		int32_t m_lastActiveSystem;
	};

	template <size_t MaxParticleSystems>
	void UParticleSystemManager::UParticleSystemManager() : m_lastActiveSystem(-1) {}

	template <size_t MaxParticleSystems>
	void UParticleSystemManager::UpdateParticleSystems(float inDeltaTime)
	{
		for (int32_t i = 0; i < m_lastActiveSystem; ++i)
		{
			m_particleSystemPool[i]
		}
	}
}