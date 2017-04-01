#pragma once

#include "Particle.h"
#include "Rendering/RenderingCommon.h"
#include "Rendering/VertexArray.h"
#include "Rendering/RenderPassDescriptor.h"
#include "Rendering/GraphicsDriverTypes.h"
#include "Rendering/ParticleSystem/ParticleSystemEmitter.h"

#include <EASTL/array.h>

namespace MAD
{
	struct SParticleSystemSpawnParams
	{
		eastl::string SystemName;
		eastl::string SystemRenderProgramPath;
		eastl::string ParticleTexturePath;
	};

	class UParticleSystem
	{
	public:
		static const uint32_t s_maxNumParticles = 4096; // uint32_t because DX API takes a UINT max
		static const size_t s_maxNumEmitters = 10;
	public:
		UParticleSystem() {}

		void Initialize(const SParticleSystemSpawnParams& inSystemParams, const eastl::vector<SParticleEmitterSpawnParams>& inEmitterParams);
		bool ActivateEmitter(const SParticleEmitterSpawnParams& inSpawnParams);

		void TickSystem(float inDeltaTime);

		void TransformParticles(const Vector3& newInitPos/*, const Vector3& newInitVel*/);
		const eastl::string& GetSystemName() const { return m_particleSystemName; }
	private:
		void InitializePipeline(const SParticleSystemSpawnParams& inSystemParams);
		void ActivateParticles(const eastl::vector<SCPUParticle>& inNewParticles);

		void UpdatePipelineData();
		void DrawParticles(float inDeltaTime);
	private:
		float m_systemDuration;
		size_t m_firstInactiveEmitter;
		uint32_t m_firstInactiveParticle;
		eastl::string m_particleSystemName;

		eastl::array<UParticleSystemEmitter, s_maxNumEmitters> m_particleEmitters;
		eastl::array<SCPUParticle, s_maxNumParticles> m_cpuParticlePool;

		// CPU-side particle data
		eastl::array<Vector3, s_maxNumParticles> m_gpuInitPosData;
		eastl::array<Vector3, s_maxNumParticles> m_gpuInitVelData;
		eastl::array<Vector4, s_maxNumParticles> m_gpuColorData;
		eastl::array<Vector2, s_maxNumParticles> m_gpuSizeData;
		eastl::array<float, s_maxNumParticles> m_gpuAgeData;

		eastl::shared_ptr<class UInputLayout> m_particleInputLayout;
		eastl::shared_ptr<class UTexture> m_particleTexture;

		// Vertex buffers for the particle system
		UVertexArray m_initParticlePosVB;
		UVertexArray m_initParticleVelVB;
		UVertexArray m_particleColorVB;
		UVertexArray m_particleSizeVB;
		UVertexArray m_particleAgeVB;

		SRenderPassDescriptor m_renderPassDescriptor;
	};

}