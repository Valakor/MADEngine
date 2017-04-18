#include "Rendering/ParticleSystem/ParticleSystemEmitter.h"
#include "Core/GameEngine.h"
#include <stdlib.h>
#include <time.h>

namespace
{
	DirectX::SimpleMath::Vector3 ComputeRandomVelocity(float inRadius, float inMinAngle, float inMaxAngle)
	{
		const float coneHeight = 1.0;
		float u = rand() % static_cast<uint32_t>(coneHeight);
		float minAngleDegrees = MAD::ConvertToDegrees(inMinAngle);
		float maxAngleDegrees = MAD::ConvertToDegrees(inMaxAngle);
		float randAngle = minAngleDegrees + (rand() % static_cast<uint32_t>(maxAngleDegrees - minAngleDegrees));
		float heightCoeffFactor = (coneHeight - u) / coneHeight;

		float velX = heightCoeffFactor * inRadius * cos(MAD::ConvertToRadians(randAngle));
		float velY = heightCoeffFactor * inRadius * sin(MAD::ConvertToRadians(randAngle));
		float velZ = u;

		return DirectX::SimpleMath::Vector3(velX, velY, velZ);
		//float velXMag = rand() % 100 - 50; // -200,200
		//float velYMag = (rand() % 200 + 100);  // 0,400
		//float velZMag = 0;
		//return DirectX::SimpleMath::Vector3(velXMag, velYMag, velZMag);
	}
}

namespace MAD
{
	SParticleEmitterSpawnParams::SParticleEmitterSpawnParams()
		: EmitRate(0.0f)
		, EmitDuration(0.0f)
		, StartColor(1.0f, 1.0f, 1.0f, 1.0f)
		, EndColor(1.0f, 1.0f, 1.0f, 1.0f)
		, StartSize(0.0f, 0.0f)
		, EndSize(0.0f, 0.0f)
		, ParticleLifetime(0.0f)
		, ConeMinAngle(0.0f)
		, ConeMinRadius(0.0f)
		, ConeMaxAngle(360.0f)
		, ConeMaxRadius(0.0f)
	{
	}

	void UParticleSystemEmitter::Initialize(const SParticleEmitterSpawnParams& inSpawnParams)
	{
		m_emitRate = 1.0f / inSpawnParams.EmitRate; // Time required for 1 particle
		m_emitDuration = inSpawnParams.EmitDuration;
		m_startColor = inSpawnParams.StartColor;
		m_endColor = inSpawnParams.EndColor;
		m_startSize = inSpawnParams.StartSize;
		m_endSize = inSpawnParams.EndSize;
		m_particleLifetime = inSpawnParams.ParticleLifetime;
		m_particleRotation = inSpawnParams.EmitRotation;

		m_coneMinAngle = ConvertToRadians(inSpawnParams.ConeMinAngle);
		m_coneMaxAngle = ConvertToRadians(inSpawnParams.ConeMaxAngle);

		m_coneMinRadius = inSpawnParams.ConeMinRadius;
		m_coneMaxRadius = inSpawnParams.ConeMaxRadius;

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
		float currentGameTime = gEngine->GetGameTime();
		float interpolationFactor = (cos(currentGameTime) * 0.5) + 0.5;

		float currentEmitRadius = Lerp(m_coneMinRadius, m_coneMaxRadius, interpolationFactor);
		Vector2 currentSize = Lerp(m_startSize, m_endSize, interpolationFactor);
		Vector4 particleColor = Lerp(m_startColor, m_endColor, interpolationFactor);
		Vector3 randomVelocity = ComputeRandomVelocity(currentEmitRadius, m_coneMinAngle, m_coneMaxAngle);

		return { Vector3(), Vector3::Transform(randomVelocity, m_particleRotation), particleColor, currentSize, 0.0f, m_particleLifetime };
	}
}

