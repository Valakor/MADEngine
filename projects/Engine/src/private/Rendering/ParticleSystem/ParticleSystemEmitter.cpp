#include "Rendering/ParticleSystem/ParticleSystemEmitter.h"

namespace MAD
{
	size_t UParticleSystemEmitter::TickEmitter(float, eastl::vector<SGPUParticle>&)
	{
		// Based on the emitter properties, potentially emit particles
		return 0;
	}
}

