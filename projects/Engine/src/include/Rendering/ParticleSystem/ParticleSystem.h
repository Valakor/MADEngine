#pragma once

#include "Particle.h"
#include "Rendering/VertexArray.h"

#include <EASTL/vector.h>

namespace MAD
{
	class UParticleSystem
	{
	public:
		void TickParticleSystem(float inDeltaTime);
	private:
		float m_systemDuration;

		eastl::vector<class UParticleSystemEmitter*> m_particleEmitters;

		eastl::vector<SGPUParticle> m_particlePool;

		// Vertex buffers for the particle system
	};
}