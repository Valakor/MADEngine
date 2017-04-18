#pragma once

#include "Particle.h"

#include <EASTL/vector.h>

namespace MAD
{
	struct SParticleEmitterSpawnParams
	{
		SParticleEmitterSpawnParams();

		uint32_t EmitRate;
		float EmitDuration;
		Color StartColor;
		Color EndColor;
		Vector2 StartSize;
		Vector2 EndSize;
		float ParticleLifetime;
		float ConeMinAngle;
		float ConeMaxAngle;
		float ConeMinRadius;
		float ConeMaxRadius;
		Matrix EmitRotation;
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
		Color m_startColor;
		Color m_endColor;
		Vector2 m_startSize;
		Vector2 m_endSize;
		float m_particleLifetime;
		float m_coneMinAngle;
		float m_coneMinRadius;
		float m_coneMaxAngle;
		float m_coneMaxRadius;
		Matrix m_particleRotation;
		bool m_bRepeat;
	};
}