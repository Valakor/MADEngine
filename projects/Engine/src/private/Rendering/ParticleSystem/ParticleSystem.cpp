#include "Rendering/ParticleSystem/ParticleSystem.h"
#include "Rendering/ParticleSystem/ParticleSystemEmitter.h"
#include "Rendering/RenderContext.h"
#include "Rendering/Renderer.h"
#include "Rendering/GraphicsDriver.h"
#include "Rendering/InputLayout.h"
#include "Rendering/InputLayoutElementArray.h"
#include "Rendering/RenderPassProgram.h"
#include "Rendering/Texture.h"

#include "Misc/AssetCache.h"
#include "Misc/Logging.h"

namespace MAD
{
	DECLARE_LOG_CATEGORY(LogParticleSystem);

	void UParticleSystem::Initialize(const SParticleSystemSpawnParams& inSystemParams, const eastl::vector<SParticleEmitterSpawnParams>& inEmitterParams)
	{
		m_particleSystemName = inSystemParams.SystemName;

		InitializePipeline(inSystemParams);

		m_firstInactiveEmitter = 0;
		m_firstInactiveParticle = 0;

		for (const auto& currEmitterSpawnParams : inEmitterParams)
		{
			ActivateEmitter(currEmitterSpawnParams);
		}
	}

	bool UParticleSystem::ActivateEmitter(const SParticleEmitterSpawnParams& inSpawnParams)
	{
		if (m_firstInactiveEmitter == UParticleSystem::s_maxNumEmitters)
		{
			return false;
		}

		++m_firstInactiveEmitter;

		m_particleEmitters[m_firstInactiveEmitter - 1].Initialize(inSpawnParams);

		return true;
	}

	void UParticleSystem::TransformParticles(const Vector3& newInitPos/*, const Vector3& newInitVel*/)
	{
		for (size_t i = 0; i < m_firstInactiveParticle; ++i)
		{
			m_cpuParticlePool[i].InitialPosVS = newInitPos;
		}
	}

	void UParticleSystem::TickSystem(float inDeltaTime)
	{
		eastl::vector<SCPUParticle> particleBuffer;

		// Allow the particle emitters to emit particles if needed
		for (size_t i = 0; i < m_firstInactiveEmitter; ++i)
		{
			m_particleEmitters[i].TickEmitter(inDeltaTime, particleBuffer);

			if (!particleBuffer.empty())
			{
				// TODO: We need to retrieve the correct initial view space position of owning component so we can set initial vs position correctly
				ActivateParticles(particleBuffer);
			}

			particleBuffer.clear();
		}

		// Kill off particles if needed
		const size_t numActiveParticles = m_firstInactiveParticle;
		for (size_t i = 0; i < numActiveParticles; ++i)
		{
			if (m_cpuParticlePool[i].ParticleAge > m_cpuParticlePool[i].Duration)
			{
				eastl::swap(m_cpuParticlePool[i], m_cpuParticlePool[m_firstInactiveParticle - 1]);

				--m_firstInactiveParticle;
			}
		}

		// Tick all of the active particles
		for (size_t i = 0; i < m_firstInactiveParticle; ++i)
		{
			m_cpuParticlePool[i].ParticleAge += inDeltaTime;
		}

		//LOG(LogParticleSystem, Log, "Active Particles: %d\n", m_firstInactiveParticle);

		// Perform draw calls on the particles that are still alive
		DrawParticles(inDeltaTime);
	}

	void UParticleSystem::InitializePipeline(const SParticleSystemSpawnParams& inSystemParams)
	{
		eastl::vector<char> compiledByteCode;
		auto& renderer = URenderContext::Get().GetRenderer();
		auto& graphicsDriver = URenderContext::Get().GetGraphicsDriver();
		const auto& gBufferPassDesc = renderer.GetGBufferPassDescriptor();

		if (!graphicsDriver.CompileShaderFromFile(UAssetCache::GetAssetRoot() + inSystemParams.SystemRenderProgramPath, "VS", "vs_5_0", compiledByteCode))
		{
			MAD_ASSERT_DESC(false, "Error compiling particle system shader");
			return;
		}

		// Initialize render pass descriptor (we need to make sure that we use the g-buffer depth buffer
		m_renderPassDescriptor.m_renderTargets.clear();
		m_renderPassDescriptor.m_renderTargets.push_back(gBufferPassDesc.m_renderTargets[AsIntegral(ERenderTargetSlot::LightingBuffer)]);

		// Reuse the G-Buffer depth stencil view so that our particles obey regular occlusion logic with existing objects in scene
		m_renderPassDescriptor.m_depthStencilState = gBufferPassDesc.m_depthStencilState;
		m_renderPassDescriptor.m_depthStencilView = gBufferPassDesc.m_depthStencilView;

		m_renderPassDescriptor.m_rasterizerState = graphicsDriver.CreateRasterizerState(D3D11_FILL_SOLID, D3D11_CULL_BACK);
		m_renderPassDescriptor.m_blendState = graphicsDriver.CreateBlendState(false/*, D3D11_BLEND_SRC_COLOR*/); // TODO Change blend state creation API to allow for different types of blending
		m_renderPassDescriptor.m_renderPassProgram = URenderPassProgram::Load(inSystemParams.SystemRenderProgramPath);

		// Create input layout
		UInputLayoutElementArray layoutElemArray;

		layoutElemArray.PushElement<Vector3>("POSITION", 0, AsIntegral(EParticleVertexBufferSlot::InitialPos));
		layoutElemArray.PushElement<Vector3>("VELOCITY", 0, AsIntegral(EParticleVertexBufferSlot::InitialVel));
		layoutElemArray.PushElement<Vector4>("COLOR", 0, AsIntegral(EParticleVertexBufferSlot::Color));
		layoutElemArray.PushElement<Vector2>("SIZE", 0, AsIntegral(EParticleVertexBufferSlot::Size));
		layoutElemArray.PushElement<float>("AGE", 0, AsIntegral(EParticleVertexBufferSlot::Age));

		m_particleInputLayout = eastl::make_shared<UInputLayout>(layoutElemArray, compiledByteCode.data(), compiledByteCode.size());

		// Create particle system texture
		if (!inSystemParams.ParticleTexturePath.empty())
		{
			m_particleTexture = UTexture::Load(inSystemParams.ParticleTexturePath, false, false);
		}

		// Initialize vertex buffers for particle data
		//m_initParticlePosVB = UVertexArray(graphicsDriver, AsIntegral(EParticleVertexBufferSlot::InitialPos), EInputLayoutSemantic::INVALID, nullptr, GetSize(&SCPUParticle::InitialPosVS), s_maxNumParticles, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
		m_initParticlePosVB = UVertexArray(graphicsDriver, AsIntegral(EParticleVertexBufferSlot::InitialPos), EInputLayoutSemantic::INVALID, nullptr, sizeof(SCPUParticle::InitialPosVS), s_maxNumParticles, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
		m_initParticleVelVB = UVertexArray(graphicsDriver, AsIntegral(EParticleVertexBufferSlot::InitialVel), EInputLayoutSemantic::INVALID, nullptr, sizeof(SCPUParticle::InitialVelVS), s_maxNumParticles, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
		m_particleColorVB = UVertexArray(graphicsDriver, AsIntegral(EParticleVertexBufferSlot::Color), EInputLayoutSemantic::INVALID, nullptr, sizeof(SCPUParticle::ParticleColor), s_maxNumParticles, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
		m_particleSizeVB = UVertexArray(graphicsDriver, AsIntegral(EParticleVertexBufferSlot::Size), EInputLayoutSemantic::INVALID, nullptr, sizeof(SCPUParticle::ParticleSize), s_maxNumParticles, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
		m_particleAgeVB = UVertexArray(graphicsDriver, AsIntegral(EParticleVertexBufferSlot::Age), EInputLayoutSemantic::INVALID, nullptr, sizeof(SCPUParticle::ParticleAge), s_maxNumParticles, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
	}

	void UParticleSystem::UpdatePipelineData()
	{
		auto& graphicsDriver = URenderContext::Get().GetGraphicsDriver();

		for (size_t i = 0; i < m_firstInactiveParticle; ++i)
		{
			m_gpuInitPosData[i] = m_cpuParticlePool[i].InitialPosVS;
			m_gpuInitVelData[i] = m_cpuParticlePool[i].InitialVelVS;
			m_gpuColorData[i] = m_cpuParticlePool[i].ParticleColor;
			m_gpuSizeData[i] = m_cpuParticlePool[i].ParticleSize;
			m_gpuAgeData[i] = m_cpuParticlePool[i].ParticleAge;
		}

		m_initParticlePosVB.Update(graphicsDriver, m_gpuInitPosData.data(), m_firstInactiveParticle * sizeof(SCPUParticle::InitialPosVS));
		m_initParticleVelVB.Update(graphicsDriver, m_gpuInitVelData.data(), m_firstInactiveParticle * sizeof(SCPUParticle::InitialVelVS));
		m_particleColorVB.Update(graphicsDriver, m_gpuColorData.data(), m_firstInactiveParticle * sizeof(SCPUParticle::ParticleColor));
		m_particleSizeVB.Update(graphicsDriver, m_gpuSizeData.data(), m_firstInactiveParticle * sizeof(SCPUParticle::ParticleSize));
		m_particleAgeVB.Update(graphicsDriver, m_gpuAgeData.data(), m_firstInactiveParticle * sizeof(SCPUParticle::ParticleAge));
	}

	void UParticleSystem::ActivateParticles(const eastl::vector<SCPUParticle>& inNewParticles)
	{
		if (m_firstInactiveParticle == UParticleSystem::s_maxNumParticles)
		{
			return;
		}

		size_t numSpawnedParticles = inNewParticles.size();

		if (m_firstInactiveParticle + numSpawnedParticles > UParticleSystem::s_maxNumParticles)
		{
			numSpawnedParticles -= ((m_firstInactiveParticle + numSpawnedParticles) - UParticleSystem::s_maxNumParticles);
		}

		for (size_t i = 0; i < numSpawnedParticles; ++i)
		{
			m_cpuParticlePool[m_firstInactiveParticle] = inNewParticles[i];
			++m_firstInactiveParticle;
		}
	}

	void UParticleSystem::DrawParticles(float)
	{
		auto& graphicsDriver = URenderContext::Get().GetGraphicsDriver();
		auto renderContext = graphicsDriver.TEMPGetDeviceContext();

		if (m_firstInactiveParticle == 0)
		{
			// No particles to draw, exit early
			return;
		}

		// Have to update the pipeline data every frame because we update the age of the particle each frame...not very satisfied with this, but will do for now
		UpdatePipelineData();

		// Set primitive topology
		renderContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

		// Set the input layout
		m_particleInputLayout->BindToPipeline();

		// Set particle texture (if there is one)
		if (m_particleTexture)
		{
			graphicsDriver.SetPixelShaderResource(m_particleTexture->GetTexureResource(), ETextureSlot::DiffuseMap);
		}

		// Set the vertex buffers
		m_initParticlePosVB.Bind(graphicsDriver, 0);
		m_initParticleVelVB.Bind(graphicsDriver, 0);
		m_particleColorVB.Bind(graphicsDriver, 0);
		m_particleSizeVB.Bind(graphicsDriver, 0);
		m_particleAgeVB.Bind(graphicsDriver, 0);

		// Activate the render pass descriptor
		m_renderPassDescriptor.m_renderPassProgram->SetProgramActive(graphicsDriver, 0);
		m_renderPassDescriptor.ApplyPassState(graphicsDriver);

		// Draw the particles
		renderContext->Draw(static_cast<UINT>(m_firstInactiveParticle), 0);
	}

}
