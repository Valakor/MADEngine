#include "Rendering/ParticleSystem/ParticleSystem.h"
#include "Rendering/ParticleSystem/ParticleSystemEmitter.h"

namespace MAD
{
	void UParticleSystem::TickParticleSystem(float inDeltaTime)
	{
		// Allow the particle emitters to emit particles if needed
		for (UParticleSystemEmitter* currentEmitter : m_particleEmitters)
		{
			currentEmitter->TickEmitter(inDeltaTime, m_particlePool);
		}
	}
}
