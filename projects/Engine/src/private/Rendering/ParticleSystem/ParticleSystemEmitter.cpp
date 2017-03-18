#include "Rendering/ParticleSystem/ParticleSystemEmitter.h"
#include "Core/GameEngine.h"
#include <stdlib.h>
#include <time.h>

namespace MAD
{
	SParticleEmitterSpawnParams::SParticleEmitterSpawnParams(float inRate, float inDuration) : EmitRate(inRate), EmitDuration(inDuration) {}

	void UParticleSystemEmitter::Initialize(const SParticleEmitterSpawnParams& inSpawnParams)
	{
		m_emitRate = 1.0f / inSpawnParams.EmitRate;
		m_emitDuration = inSpawnParams.EmitDuration;
		m_rateAccumulator = 0.0f;
		m_runningEmitDuration = 0.0f;
		m_bRepeat = (inSpawnParams.EmitDuration == -1.0f);
	}

	void UParticleSystemEmitter::TickEmitter(float inDeltaTime, eastl::vector<SCPUParticle>& outEmittedParticles)
	{
		if (IsFinished())
		{
			return;
		}

		m_runningEmitDuration += inDeltaTime;
		m_rateAccumulator += inDeltaTime;

		// Accumulated enough time to emit a particle
		if (m_rateAccumulator >= m_emitRate)
		{
			outEmittedParticles.emplace_back(EmitParticle());

			m_rateAccumulator -= m_emitRate;
		}
	}

	bool UParticleSystemEmitter::IsFinished() const
	{
		return m_runningEmitDuration > m_emitDuration && !m_bRepeat;
	}

	SCPUParticle UParticleSystemEmitter::EmitParticle()
	{
		// Emit particle with random velocity and color
		return { Vector3(), Vector3(rand() % 100, rand() % 100, rand() % 100), Vector4(0.0f, 1.0f, 0.0f, 1.0f), Vector2(1.0f, 1.0f), gEngine->GetGameTime() + 1.0f };
	}
}

