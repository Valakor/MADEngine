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
		if (!m_bEnabled)
		{
			return;
		}

		// Request a particle system from the renderer based on spawn parameters
		SParticleSystemSpawnParams systemSpawnParams;
		eastl::vector<SParticleEmitterSpawnParams> emitterSpawnParams; // Only one for now

		systemSpawnParams.SystemName = m_systemName;
		systemSpawnParams.SystemRenderProgramPath = m_systemEffectProgramPath;
		systemSpawnParams.ParticleTexturePath = m_systemEffectTexturePath;

		emitterSpawnParams.emplace_back(50, -1.0f); // repeat at 500 particles/second

		m_particleSystem = URenderContext::Get().GetRenderer().SpawnParticleSystem(systemSpawnParams, emitterSpawnParams);

		// Set particle initial translation data
		m_particleSystem->TransformParticles(GetViewSpacePosition());
	}

	void CParticleSystemComponent::Load(const UGameWorldLoader& inLoader)
	{
		// Load in particle system and emitter data
		// ***Testing loading code***
		UArrayValue emitterArray;

		inLoader.GetBool("enabled", m_bEnabled);
		inLoader.GetString("name", m_systemName);
		inLoader.GetString("effect_shader", m_systemEffectProgramPath);
		inLoader.GetString("effect_texture", m_systemEffectTexturePath);

		if (inLoader.GetArray("emitters", emitterArray))
		{
			const SizeType numEmitters = emitterArray.Size();

			for (SizeType i = 0; i < numEmitters; ++i)
			{
				UObjectValue currentEmitter = emitterArray[i];

				UObjectValue emitObject;
				eastl::string property1;
				eastl::string property2;
				uint32_t emitRate = 0;
				float emitDuration = 0.0f;

				currentEmitter.GetProperty("emit_rate", emitRate);
				currentEmitter.GetProperty("emit_duration", emitDuration);

				if (currentEmitter.GetProperty("emit_object", emitObject))
				{
					UArrayValue propertyListArray;

					emitObject.GetProperty("property1", property1);
					emitObject.GetProperty("property2", property2);

					if (emitObject.GetProperty("propertyList", propertyListArray))
					{
						UObjectValue currentPropertyObj;

						for (SizeType j = 0; j < propertyListArray.Size(); ++j)
						{
							if (propertyListArray[j].Get(currentPropertyObj))
							{
								Vector3 propertyA;
								eastl::string propertyB;

								currentPropertyObj.GetProperty("propertyA", propertyA);
								currentPropertyObj.GetProperty("propertyB", propertyB);
							}
						}
					}
				}
			}
		}
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
		const Vector3& entityWorldTranslation = GetOwningEntity().GetWorldTranslation();

		return Vector3::Transform(entityWorldTranslation, cameraViewMatrix);
	}

}