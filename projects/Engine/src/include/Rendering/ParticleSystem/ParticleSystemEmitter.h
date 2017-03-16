#pragma once

#include "Particle.h"

#include <EASTL/vector.h>

namespace MAD
{
	class UParticleSystemEmitter
	{
	public:
		 size_t TickEmitter(float inDeltaTime, eastl::vector<SGPUParticle>& inOutParticlePool);
	private:
		bool m_bSpawnOnRepeat;
		float m_emitRate; // particles-per-second
		float m_emitLifetime;
	};
}