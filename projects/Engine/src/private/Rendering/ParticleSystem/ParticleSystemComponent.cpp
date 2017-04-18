#include "Rendering/ParticleSystem/ParticleSystemComponent.h"

#include "Core/Pipeline/GameWorldLoader.h"

#include "Rendering/RenderContext.h"
#include "Rendering/Renderer.h"
#include "Rendering/ParticleSystem/ParticleSystem.h"

#include "Misc/Assert.h"

namespace MAD
{
	CParticleSystemComponent::CParticleSystemComponent(OGameWorld* inOwningWorld) : Super_t(inOwningWorld), m_bEnabled(true), m_particleSystem(nullptr)
	{
	}

	void CParticleSystemComponent::PostInitializeComponents()
	{
		if (!m_bEnabled || !m_particleSystem)
		{
			return;
		}

		// Set particle initial translation data
		m_particleSystem->TransformParticles(GetViewSpacePosition());
	}

	void CParticleSystemComponent::Load(const UGameWorldLoader& inLoader, const UObjectValue& inPropertyObj)
	{
		UNREFERENCED_PARAMETER(inLoader);

		// Request a particle system from the renderer based on spawn parameters
		SParticleSystemSpawnParams systemSpawnParams;
		eastl::vector<SParticleEmitterSpawnParams> emitterSpawnParams;

		// Load in particle system and emitter data
		UArrayValue emitterArray;

		inPropertyObj.GetProperty("enabled", m_bEnabled);

		if (inPropertyObj.GetProperty("name", m_systemName))
		{
			systemSpawnParams.SystemName = m_systemName;
		}

		if (inPropertyObj.GetProperty("effect_shader", m_systemEffectProgramPath))
		{
			systemSpawnParams.SystemRenderProgramPath = m_systemEffectProgramPath;
		}

		if (inPropertyObj.GetProperty("effect_texture", m_systemEffectTexturePath))
		{
			systemSpawnParams.ParticleTexturePath = m_systemEffectTexturePath;
		}

		if (inPropertyObj.GetProperty("emitters", emitterArray))
		{
			const SizeType numEmitters = emitterArray.Size();

			for (SizeType i = 0; i < numEmitters; ++i)
			{
				UObjectValue currentEmitter = emitterArray[i];
				SParticleEmitterSpawnParams currentEmitterParams;

				currentEmitter.GetProperty("emit_rate", currentEmitterParams.EmitRate);
				currentEmitter.GetProperty("emit_duration", currentEmitterParams.EmitDuration);
				currentEmitter.GetProperty("emit_start_color", currentEmitterParams.StartColor);

				if (!currentEmitter.GetProperty("emit_end_color", currentEmitterParams.EndColor))
				{
					currentEmitterParams.EndColor = currentEmitterParams.StartColor;
				}

				currentEmitter.GetProperty("emit_start_size", currentEmitterParams.StartSize);

				if (!currentEmitter.GetProperty("emit_end_size", currentEmitterParams.EndSize))
				{
					currentEmitterParams.EndSize = currentEmitterParams.StartSize;
				}

				currentEmitter.GetProperty("emit_min_angle", currentEmitterParams.ConeMinAngle);
				currentEmitter.GetProperty("emit_max_angle", currentEmitterParams.ConeMaxAngle);

				currentEmitter.GetProperty("emit_min_radius", currentEmitterParams.ConeMinRadius);
				if (!currentEmitter.GetProperty("emit_max_radius", currentEmitterParams.ConeMaxRadius))
				{
					currentEmitterParams.ConeMaxRadius = currentEmitterParams.ConeMinRadius;
				}

				currentEmitter.GetProperty("emit_particle_lifetime", currentEmitterParams.ParticleLifetime);

				Vector3 eulerRotationAngles;
				if (currentEmitter.GetProperty("emit_rotation", eulerRotationAngles))
				{
					currentEmitterParams.EmitRotation = Matrix::CreateFromYawPitchRoll(eulerRotationAngles.y, eulerRotationAngles.x, eulerRotationAngles.z);
				}

				emitterSpawnParams.push_back(currentEmitterParams);
			}
		}

		m_particleSystem = URenderContext::Get().GetRenderer().SpawnParticleSystem(systemSpawnParams, emitterSpawnParams);
	}

	void CParticleSystemComponent::UpdateComponent(float)
	{
		if (!m_bEnabled)
		{
			return;
		}

		m_particleSystem->TransformParticles(GetViewSpacePosition());
	}

	Vector3 CParticleSystemComponent::GetViewSpacePosition() const
	{
		// Update particle system's positional/rotational information
		const Matrix& cameraViewMatrix = URenderContext::Get().GetRenderer().GetPerFrameConstants().m_cameraViewMatrix;

		return Vector3::Transform(GetWorldTranslation(), cameraViewMatrix);
	}

}