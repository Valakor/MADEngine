#include "Rendering/ParticleSystem/ParticleSystemComponent.h"
#include "Rendering/RenderContext.h"
#include "Rendering/Renderer.h"
#include "Rendering/ParticleSystem/ParticleSystem.h"

#include "Misc/Assert.h"

namespace MAD
{
	CParticleSystemComponent::CParticleSystemComponent(OGameWorld* inOwningWorld) : Super_t(inOwningWorld), m_particleSystem(nullptr)
	{
	}

	void CParticleSystemComponent::PostInitializeComponents()
	{
		// Request a particle system from the renderer based on spawn parameters
		SParticleSystemSpawnParams systemSpawnParams;
		eastl::vector<SParticleEmitterSpawnParams> emitterSpawnParams; // Only one for now

		systemSpawnParams.SystemName = "Basic Particles";
		systemSpawnParams.SystemRenderProgramPath = "engine\\shaders\\ParticleSystem\\ParticleSystemCPU.hlsl";
		systemSpawnParams.ParticleTexturePath = "engine\\textures\\full_fire.jpg";

		emitterSpawnParams.emplace_back(1000, -1.0f); // repeat at 500 particles/second

		m_particleSystem = URenderContext::Get().GetRenderer().SpawnParticleSystem(systemSpawnParams, emitterSpawnParams);

		// Set particle initial translation data
		m_particleSystem->TransformParticles(GetViewSpacePosition());
	}

	void CParticleSystemComponent::Load(const UGameWorldLoader&)
	{
		// Load in particle system and emitter data
	}

	void CParticleSystemComponent::UpdateComponent(float)
	{
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