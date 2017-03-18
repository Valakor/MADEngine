#include "Rendering/ParticleSystem/ParticleSystem.h"
#include "Rendering/ParticleSystem/ParticleSystemEmitter.h"
#include "Rendering/RenderContext.h"
#include "Rendering/Renderer.h"
#include "Rendering/GraphicsDriver.h"
#include "Rendering/InputLayout.h"
#include "Rendering/InputLayoutElementArray.h"
#include "Rendering/RenderPassProgram.h"

#include "Misc/AssetCache.h"
#include "Misc/Logging.h"

namespace MAD
{
	DECLARE_LOG_CATEGORY(LogParticleSystem);

	template <typename T, typename M>
	M get_type_of(M T::*);

	#define GetTypeOf(MemVar)	decltype(get_type_of(&MemVar))
	#define GetSizeOf(MemVar)		sizeof(GetTypeOf(MemVar))

	void UParticleSystem::Initialize(const SParticleSystemSpawnParams& inSystemParams, const eastl::vector<SParticleEmitterSpawnParams>& inEmitterParams)
	{
		m_particleSystemName = inSystemParams.SystemName;

		InitializePipeline(inSystemParams.SystemRenderProgramPath);

		m_bIsDirty = false;
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

		if (m_firstInactiveParticle > 0)
		{
			m_bIsDirty = true;
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
			if (m_cpuParticlePool[i].Age > m_cpuParticlePool[i].Duration)
			{
				eastl::swap(m_cpuParticlePool[i], m_cpuParticlePool[m_firstInactiveParticle - 1]);

				--m_firstInactiveParticle;

				m_bIsDirty = true;
			}
		}

		// Tick all of the active particles
		for (size_t i = 0; i < m_firstInactiveParticle; ++i)
		{
			m_cpuParticlePool[i].Age += inDeltaTime;
		}

		//LOG(LogParticleSystem, Log, "Active Particles: %d\n", m_firstInactiveParticle);

		// Perform draw calls on the particles that are still alive
		DrawParticles(inDeltaTime);

		m_bIsDirty = false;
	}

	void UParticleSystem::InitializePipeline(const eastl::string& inRenderPassPath)
	{
		eastl::vector<char> compiledByteCode;
		auto& renderer = URenderContext::Get().GetRenderer();
		auto& graphicsDriver = URenderContext::Get().GetGraphicsDriver();
		const auto& gBufferPassDesc = renderer.GetGBufferPassDescriptor();

		if (!graphicsDriver.CompileShaderFromFile(UAssetCache::GetAssetRoot() + inRenderPassPath, "VS", "vs_5_0", compiledByteCode))
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
		m_renderPassDescriptor.m_blendState = graphicsDriver.CreateBlendState(true); // TODO Change blend state creation API to allow for different types of blending
		m_renderPassDescriptor.m_renderPassProgram = URenderPassProgram::Load(inRenderPassPath);

		// Create input layout
		UInputLayoutElementArray layoutElemArray;

		layoutElemArray.PushElement<Vector3>("POSITION", 0, AsIntegral(EParticleVertexBufferSlot::InitialPos));
		layoutElemArray.PushElement<Vector3>("VELOCITY", 0, AsIntegral(EParticleVertexBufferSlot::InitialVel));
		layoutElemArray.PushElement<Vector4>("COLOR", 0, AsIntegral(EParticleVertexBufferSlot::Color));
		layoutElemArray.PushElement<Vector2>("SIZE", 0, AsIntegral(EParticleVertexBufferSlot::Size));
		layoutElemArray.PushElement<float>("AGE", 0, AsIntegral(EParticleVertexBufferSlot::Age));

		m_particleInputLayout = eastl::make_shared<UInputLayout>(layoutElemArray, compiledByteCode.data(), compiledByteCode.size());

		// Initialize vertex buffers for particle data
		m_initialPosVB = graphicsDriver.CreateVertexBuffer(nullptr, s_maxNumParticles * GetSizeOf(SCPUParticle::InitialPosVS), D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
		m_initialVelVB = graphicsDriver.CreateVertexBuffer(nullptr, s_maxNumParticles * GetSizeOf(SCPUParticle::InitialVelVS), D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
		m_particleColorVB = graphicsDriver.CreateVertexBuffer(nullptr, s_maxNumParticles * GetSizeOf(SCPUParticle::ParticleColor), D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
		m_particleSizeVB = graphicsDriver.CreateVertexBuffer(nullptr, s_maxNumParticles * GetSizeOf(SCPUParticle::ParticleSize), D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
		m_particleAgeVB = graphicsDriver.CreateVertexBuffer(nullptr, s_maxNumParticles * GetSizeOf(SCPUParticle::Age), D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
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
			m_gpuAgeData[i] = m_cpuParticlePool[i].Age;
		}

		graphicsDriver.UpdateBuffer(m_initialPosVB, m_gpuInitPosData.data(), m_firstInactiveParticle * GetSizeOf(SCPUParticle::InitialPosVS));
		graphicsDriver.UpdateBuffer(m_initialVelVB, m_gpuInitVelData.data(), m_firstInactiveParticle * GetSizeOf(SCPUParticle::InitialVelVS));
		graphicsDriver.UpdateBuffer(m_particleColorVB, m_gpuColorData.data(), m_firstInactiveParticle * GetSizeOf(SCPUParticle::ParticleColor));
		graphicsDriver.UpdateBuffer(m_particleSizeVB, m_gpuSizeData.data(), m_firstInactiveParticle * GetSizeOf(SCPUParticle::ParticleSize));
		graphicsDriver.UpdateBuffer(m_particleAgeVB, m_gpuAgeData.data(), m_firstInactiveParticle * GetSizeOf(SCPUParticle::Age));
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

		if (numSpawnedParticles > 0)
		{
			m_bIsDirty = true;
		}
	}

	void UParticleSystem::DrawParticles(float)
	{
		const UINT initialPosSize = GetSizeOf(SCPUParticle::InitialPosVS);
		const UINT initialVelSize = GetSizeOf(SCPUParticle::InitialVelVS);
		const UINT particleColorSize = GetSizeOf(SCPUParticle::ParticleColor);
		const UINT particleExtentSize = GetSizeOf(SCPUParticle::ParticleSize);
		const UINT particleAgeSize = GetSizeOf(SCPUParticle::Age);

		const UINT byteOffset = 0;

		auto& graphicsDriver = URenderContext::Get().GetGraphicsDriver();
		auto renderContext = graphicsDriver.TEMPGetDeviceContext();

		if (m_firstInactiveParticle == 0)
		{
			// No particles to draw, exit early
			return;
		}

		/*if (m_bIsDirty)
		{
			// GPU particle data is dirty, update vertex buffers
		}*/

		// Have to update the pipeline data every frame because we update the age of the particle each frame...not very satisfied with this, but will do for now
		UpdatePipelineData();

		// Set primitive topology
		renderContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

		// Set the input layout
		m_particleInputLayout->BindToPipeline();

		// Set the vertex buffers
		renderContext->IASetVertexBuffers(AsIntegral(EParticleVertexBufferSlot::InitialPos), 1, m_initialPosVB.GetAddressOf(), &initialPosSize, &byteOffset);
		renderContext->IASetVertexBuffers(AsIntegral(EParticleVertexBufferSlot::InitialVel), 1, m_initialVelVB.GetAddressOf(), &initialVelSize, &byteOffset);
		renderContext->IASetVertexBuffers(AsIntegral(EParticleVertexBufferSlot::Color), 1, m_particleColorVB.GetAddressOf(), &particleColorSize, &byteOffset);
		renderContext->IASetVertexBuffers(AsIntegral(EParticleVertexBufferSlot::Size), 1, m_particleSizeVB.GetAddressOf(), &particleExtentSize, &byteOffset);
		renderContext->IASetVertexBuffers(AsIntegral(EParticleVertexBufferSlot::Age), 1, m_particleAgeVB.GetAddressOf(), &particleAgeSize, &byteOffset);

		// Activate the render pass descriptor
		m_renderPassDescriptor.m_renderPassProgram->SetProgramActive(graphicsDriver, 0);
		m_renderPassDescriptor.ApplyPassState(graphicsDriver);

		// Draw the particles
		renderContext->Draw(static_cast<UINT>(m_firstInactiveParticle), 0);
	}

}
