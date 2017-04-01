#pragma once

#include "Particle.h"

#include <EASTL/vector.h>

namespace MAD
{
	struct SParticleEmitterSpawnParams
	{
		SParticleEmitterSpawnParams(float inRate, float inDuration);

		int32_t EmitRate;
		float EmitDuration;
	};

	class UParticleSystemEmitter
	{
	public:
		void Initialize(const SParticleEmitterSpawnParams& inSpawnParams);
		void TickEmitter(float inDeltaTime, eastl::vector<SCPUParticle>& outEmittedParticles);

		bool IsFinished() const;
	private:
		SCPUParticle EmitParticle();
	private:
		float m_emitRate; // seconds-per-particle
		float m_emitDuration;
		float m_rateAccumulator;
		float m_runningEmitDuration;
		bool m_bRepeat;
	};
}