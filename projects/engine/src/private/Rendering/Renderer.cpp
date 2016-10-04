#include "Rendering/Renderer.h"
#include "Rendering/RenderPassProgram.h"
#include "Rendering/CameraInstance.h"

#include "Core/GameWindow.h"
#include "Misc/ProgramPermutor.h"
#include "Misc/AssetCache.h"
#include "Misc/Logging.h"
#include "Misc/utf8conv.h"
#include "Rendering/GraphicsDriver.h"
#include "Rendering/InputLayoutCache.h"

#include <EASTL/array.h>

namespace MAD
{
	DECLARE_LOG_CATEGORY(LogRenderer);

	namespace
	{
		UGraphicsDriver g_graphicsDriver;

		void DrawFullscreenQuad()
		{
			static bool init = false;

			static SBufferId indices;
			static SBufferId verts;
			static SInputLayoutId posInputLayout;
			static SRasterizerStateId rasterState;
			static SDepthStencilStateId depthState;

			if (!init)
			{
				static const eastl::vector<Vector3> verts_raw = { { -1.0f, 1.0f, 0.0f }, { 1.0f, 1.0f, 0.0f }, { 1.0f, -1.0f, 0.0f }, { -1.0f, -1.0f, 0.0f } };
				static const eastl::vector<uint16_t> indices_raw = { 0, 3, 1, 1, 3, 2 };

				indices = g_graphicsDriver.CreateIndexBuffer(indices_raw.data(), 6 * sizeof(uint16_t));
				verts = g_graphicsDriver.CreateVertexBuffer(verts_raw.data(), 4 * sizeof(Vector3));

				posInputLayout = UInputLayoutCache::GetInputLayout(UInputLayoutCache::GetFlagForSemanticName("POSITION"));
				rasterState = g_graphicsDriver.CreateRasterizerState(D3D11_FILL_SOLID, D3D11_CULL_BACK);
				depthState = g_graphicsDriver.CreateDepthStencilState(false, D3D11_COMPARISON_ALWAYS);

				init = true;
			}

			g_graphicsDriver.SetDepthStencilState(depthState, 0);
			g_graphicsDriver.SetRasterizerState(rasterState);
			g_graphicsDriver.SetInputLayout(posInputLayout);
			g_graphicsDriver.SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			g_graphicsDriver.SetIndexBuffer(indices, 0);
			g_graphicsDriver.SetVertexBuffer(verts, EVertexBufferSlot::Position, sizeof(Vector3), 0);

			g_graphicsDriver.DrawIndexed(6, 0, 0);

			// Alternatively use the SV_VertexID semantic in the shader and calculate the coordinates like:
			//   float2 texCoord = float2(id & 1, id >> 1);
			//   output.mPos = float4((texCoord.x - 0.5f) * 2.0f, -(texCoord.y - 0.5f) * 2.0f, 0.0f, 1.0f);
			//g_graphicsDriver.SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
			//g_graphicsDriver.Draw(4, 0);
		}
	}

	URenderer::URenderer(): m_window(nullptr)
	                      , m_visualizeOption(EVisualizeOptions::None) { }

	bool URenderer::Init(UGameWindow& inWindow)
	{
		LOG(LogRenderer, Log, "Renderer initialization begin...\n");

		m_window = &inWindow;

		if (!g_graphicsDriver.Init(inWindow))
		{
			return false;
		}

		m_backBuffer = g_graphicsDriver.GetBackBufferRenderTarget();

		auto clientSize = inWindow.GetClientSize();
		SetViewport(clientSize.x, clientSize.y);

		InitializeGBufferPass("engine\\shaders\\GBuffer.hlsl");

		InitializeDirectionalLightingPass("engine\\shaders\\DeferredLighting.hlsl");

		LOG(LogRenderer, Log, "Renderer initialization successful\n");
		return true;
	}

	void URenderer::Shutdown()
	{
		g_graphicsDriver.Shutdown();
	}

	void URenderer::QueueDrawItem(const SDrawItem& inDrawItem)
	{
		m_queuedDrawItems.emplace_back(inDrawItem);
	}

	void URenderer::QueueDirectionLight(const SGPUDirectionalLight& inDirectionalLight)
	{
		using namespace DirectX::SimpleMath;
		m_queuedDirLights.push_back(inDirectionalLight);
		m_queuedDirLights.back().m_lightDirection = Vector3::TransformNormal(inDirectionalLight.m_lightDirection, m_perFrameConstants.m_cameraViewMatrix);
	}

	void URenderer::OnScreenSizeChanged()
	{
		auto newSize = m_window->GetClientSize();
		LOG(LogRenderer, Log, "OnScreenSizeChanged: { %i, %i }\n", newSize.x, newSize.y);

		g_graphicsDriver.OnScreenSizeChanged();

		InitializeGBufferPass("engine\\shaders\\GBuffer.hlsl");

		InitializeDirectionalLightingPass("engine\\shaders\\DeferredLighting.hlsl");

		auto clientSize = m_window->GetClientSize();
		SetViewport(clientSize.x, clientSize.y);

		m_backBuffer = g_graphicsDriver.GetBackBufferRenderTarget();
	}

	void URenderer::Frame(float framePercent)
	{
		(void)framePercent;
		
		BeginFrame();
		Draw();
		EndFrame();
	}

	void URenderer::InitializeGBufferPass(const eastl::string& inGBufferProgramPath)
	{
		// G-Buffer textures will be the same size as the screen
		auto clientSize = m_window->GetClientSize();

		m_gBufferShaderResources.resize(AsIntegral(ETextureSlot::MAX) - AsIntegral(ETextureSlot::LightingBuffer));
		g_graphicsDriver.SetPixelShaderResource(SShaderResourceId::Invalid, ETextureSlot::LightingBuffer);
		g_graphicsDriver.SetPixelShaderResource(SShaderResourceId::Invalid, ETextureSlot::DiffuseBuffer);
		g_graphicsDriver.SetPixelShaderResource(SShaderResourceId::Invalid, ETextureSlot::NormalBuffer);
		g_graphicsDriver.SetPixelShaderResource(SShaderResourceId::Invalid, ETextureSlot::SpecularBuffer);
		g_graphicsDriver.SetPixelShaderResource(SShaderResourceId::Invalid, ETextureSlot::DepthBuffer);
		for (unsigned i = 0; i < m_gBufferShaderResources.size(); ++i)
		{
			g_graphicsDriver.DestroyShaderResource(m_gBufferShaderResources[i]);
		}

		g_graphicsDriver.DestroyDepthStencil(m_gBufferPassDescriptor.m_depthStencilView);
		SShaderResourceId& depthBufferSRV = m_gBufferShaderResources[AsIntegral(ETextureSlot::DepthBuffer) - AsIntegral(ETextureSlot::LightingBuffer)];
		m_gBufferPassDescriptor.m_depthStencilView = g_graphicsDriver.CreateDepthStencil(clientSize.x, clientSize.y, &depthBufferSRV);
		m_gBufferPassDescriptor.m_depthStencilState = g_graphicsDriver.CreateDepthStencilState(true, D3D11_COMPARISON_LESS);

		for (unsigned i = 0; i < m_gBufferPassDescriptor.m_renderTargets.size(); ++i)
		{
			g_graphicsDriver.DestroyRenderTarget(m_gBufferPassDescriptor.m_renderTargets[i]);
		}
		
		SShaderResourceId& lightBufferSRV = m_gBufferShaderResources[AsIntegral(ETextureSlot::LightingBuffer) - AsIntegral(ETextureSlot::LightingBuffer)];
		SShaderResourceId& diffuseBufferSRV = m_gBufferShaderResources[AsIntegral(ETextureSlot::DiffuseBuffer) - AsIntegral(ETextureSlot::LightingBuffer)];
		SShaderResourceId& normalBufferSRV = m_gBufferShaderResources[AsIntegral(ETextureSlot::NormalBuffer) - AsIntegral(ETextureSlot::LightingBuffer)];
		SShaderResourceId& specularBufferSRV = m_gBufferShaderResources[AsIntegral(ETextureSlot::SpecularBuffer) - AsIntegral(ETextureSlot::LightingBuffer)];

		m_gBufferPassDescriptor.m_renderTargets.resize(AsIntegral(ERenderTargetSlot::MAX));
		m_gBufferPassDescriptor.m_renderTargets[AsIntegral(ERenderTargetSlot::LightingBuffer)] = g_graphicsDriver.CreateRenderTarget(clientSize.x, clientSize.y, DXGI_FORMAT_R16G16B16A16_FLOAT, &lightBufferSRV);
		m_gBufferPassDescriptor.m_renderTargets[AsIntegral(ERenderTargetSlot::DiffuseBuffer)] = g_graphicsDriver.CreateRenderTarget(clientSize.x, clientSize.y, DXGI_FORMAT_R8G8B8A8_UNORM, &diffuseBufferSRV);
		m_gBufferPassDescriptor.m_renderTargets[AsIntegral(ERenderTargetSlot::NormalBuffer)] = g_graphicsDriver.CreateRenderTarget(clientSize.x, clientSize.y, DXGI_FORMAT_R16G16_UNORM, &normalBufferSRV);
		m_gBufferPassDescriptor.m_renderTargets[AsIntegral(ERenderTargetSlot::SpecularBuffer)] = g_graphicsDriver.CreateRenderTarget(clientSize.x, clientSize.y, DXGI_FORMAT_R8G8B8A8_UNORM, &specularBufferSRV);
#ifdef _DEBUG
		g_graphicsDriver.SetDebugName_RenderTarget(g_graphicsDriver.GetBackBufferRenderTarget(), "Back Buffer");
		g_graphicsDriver.SetDebugName_RenderTarget(m_gBufferPassDescriptor.m_renderTargets[AsIntegral(ERenderTargetSlot::LightingBuffer)], "Light Accumulation Buffer");
		g_graphicsDriver.SetDebugName_RenderTarget(m_gBufferPassDescriptor.m_renderTargets[AsIntegral(ERenderTargetSlot::DiffuseBuffer)], "Diffuse Buffer");
		g_graphicsDriver.SetDebugName_RenderTarget(m_gBufferPassDescriptor.m_renderTargets[AsIntegral(ERenderTargetSlot::NormalBuffer)], "Normal Buffer");
		g_graphicsDriver.SetDebugName_RenderTarget(m_gBufferPassDescriptor.m_renderTargets[AsIntegral(ERenderTargetSlot::SpecularBuffer)], "Specular Buffer");
#endif

		m_gBufferPassDescriptor.m_rasterizerState = g_graphicsDriver.CreateRasterizerState(D3D11_FILL_SOLID, D3D11_CULL_BACK);

		m_gBufferPassDescriptor.m_renderPassProgram = UAssetCache::Load<URenderPassProgram>(inGBufferProgramPath);

		m_gBufferPassDescriptor.m_blendState = g_graphicsDriver.CreateBlendState(false);
	}

	void URenderer::InitializeDirectionalLightingPass(const eastl::string& inDirLightingPassProgramPath)
	{
		m_dirLightingPassDescriptor.m_depthStencilView = SDepthStencilId::Invalid;
		m_dirLightingPassDescriptor.m_depthStencilState = g_graphicsDriver.CreateDepthStencilState(false, D3D11_COMPARISON_ALWAYS);

		m_dirLightingPassDescriptor.m_renderTargets.push_back(m_gBufferPassDescriptor.m_renderTargets[AsIntegral(ERenderTargetSlot::LightingBuffer)]);

		m_dirLightingPassDescriptor.m_rasterizerState = g_graphicsDriver.CreateRasterizerState(D3D11_FILL_SOLID, D3D11_CULL_FRONT);

		m_dirLightingPassDescriptor.m_renderPassProgram = UAssetCache::Load<URenderPassProgram>(inDirLightingPassProgramPath);

		m_dirLightingPassDescriptor.m_blendState = g_graphicsDriver.CreateBlendState(true);
	}

	void URenderer::BindPerFrameConstants()
	{
		// Update the per frame constant buffer
		g_graphicsDriver.UpdateBuffer(EConstantBufferSlot::PerFrame, &m_perFrameConstants, sizeof(m_perFrameConstants));
	}

	void URenderer::SetViewport(LONG inWidth, LONG inHeight)
	{
		float w = static_cast<float>(inWidth);
		float h = static_cast<float>(inHeight);

		m_perSceneConstants.m_screenDimensions.x = w;
		m_perSceneConstants.m_screenDimensions.y = h;
		g_graphicsDriver.UpdateBuffer(EConstantBufferSlot::PerScene, &m_perSceneConstants, sizeof(m_perSceneConstants));

		g_graphicsDriver.SetViewport(0, 0, w, h);
	}

	void URenderer::BeginFrame()
	{
		static const float zero[] = { 0.0f, 0.0f, 0.0f, 0.0f };

		g_graphicsDriver.ClearDepthStencil(m_gBufferPassDescriptor.m_depthStencilView, true, 1.0f);

		// Clear the light accumulation buffer to the screen clear color
		g_graphicsDriver.ClearRenderTarget(m_gBufferPassDescriptor.m_renderTargets[0], m_clearColor);

		// Clear other G-buffers to 0
		for (unsigned i = 1; i < m_gBufferPassDescriptor.m_renderTargets.size(); ++i)
		{
			auto renderTarget = m_gBufferPassDescriptor.m_renderTargets[i];
			g_graphicsDriver.ClearRenderTarget(renderTarget, zero);
		}
	}

	void URenderer::Draw()
	{
		// Bind per-frame constants
		BindPerFrameConstants();

		g_graphicsDriver.SetPixelShaderResource(SShaderResourceId::Invalid, ETextureSlot::LightingBuffer);
		g_graphicsDriver.SetPixelShaderResource(SShaderResourceId::Invalid, ETextureSlot::DiffuseBuffer);
		g_graphicsDriver.SetPixelShaderResource(SShaderResourceId::Invalid, ETextureSlot::NormalBuffer);
		g_graphicsDriver.SetPixelShaderResource(SShaderResourceId::Invalid, ETextureSlot::SpecularBuffer);
		g_graphicsDriver.SetPixelShaderResource(SShaderResourceId::Invalid, ETextureSlot::DepthBuffer);

		m_gBufferPassDescriptor.ApplyPassState(g_graphicsDriver);

		// Go through each draw item and bind input assembly data
		for (const SDrawItem& currentDrawItem : m_queuedDrawItems)
		{
			// Before processing the draw item, we need to determine which program it should use and bind that
			m_gBufferPassDescriptor.m_renderPassProgram->SetProgramActive(g_graphicsDriver, DetermineProgramId(currentDrawItem));

			// Each individual DrawItem should issue it's own draw call
			currentDrawItem.Draw(g_graphicsDriver, true);
		}

		if (m_visualizeOption != EVisualizeOptions::None)
		{
			DoVisualizeGBuffer();
			return;
		}

		// Do directional lighting
		m_dirLightingPassDescriptor.ApplyPassState(g_graphicsDriver);
		m_dirLightingPassDescriptor.m_renderPassProgram->SetProgramActive(g_graphicsDriver, 0);
		g_graphicsDriver.SetPixelShaderResource(m_gBufferShaderResources[AsIntegral(ETextureSlot::DiffuseBuffer) - AsIntegral(ETextureSlot::LightingBuffer)], ETextureSlot::DiffuseBuffer);
		g_graphicsDriver.SetPixelShaderResource(m_gBufferShaderResources[AsIntegral(ETextureSlot::NormalBuffer) - AsIntegral(ETextureSlot::LightingBuffer)], ETextureSlot::NormalBuffer);
		g_graphicsDriver.SetPixelShaderResource(m_gBufferShaderResources[AsIntegral(ETextureSlot::SpecularBuffer) - AsIntegral(ETextureSlot::LightingBuffer)], ETextureSlot::SpecularBuffer);
		g_graphicsDriver.SetPixelShaderResource(m_gBufferShaderResources[AsIntegral(ETextureSlot::DepthBuffer) - AsIntegral(ETextureSlot::LightingBuffer)], ETextureSlot::DepthBuffer);

		for (const SGPUDirectionalLight& currentDirLight : m_queuedDirLights)
		{
			g_graphicsDriver.UpdateBuffer(EConstantBufferSlot::PerDirectionalLight, &currentDirLight, sizeof(SGPUDirectionalLight));
			DrawFullscreenQuad();
		}
	}

	void URenderer::EndFrame()
	{
		static bool loadedBackBufferProgram = false;
		static eastl::shared_ptr<URenderPassProgram> backBufferProgram;
		if (!loadedBackBufferProgram)
		{
			backBufferProgram = UAssetCache::Load<URenderPassProgram>("engine\\shaders\\BackBufferFinalize.hlsl");
			loadedBackBufferProgram = true;
		}

		// Copy the finalized linear lighting buffer to the back buffer
		// This (will) perform HDR lighting corrections and already performs gamma correction
		g_graphicsDriver.SetRenderTargets(&m_backBuffer, 1, nullptr);
		g_graphicsDriver.SetPixelShaderResource(m_gBufferShaderResources[AsIntegral(ETextureSlot::LightingBuffer) - AsIntegral(ETextureSlot::LightingBuffer)], ETextureSlot::LightingBuffer);
		g_graphicsDriver.SetBlendState(SBlendStateId::Invalid);
		backBufferProgram->SetProgramActive(g_graphicsDriver, 0);

		DrawFullscreenQuad();

		g_graphicsDriver.Present();
	}

	void URenderer::DoVisualizeGBuffer()
	{
		static bool loadedCopyTextureProgram = false;
		static eastl::shared_ptr<URenderPassProgram> copyTextureProgram;
		if (!loadedCopyTextureProgram)
		{
			copyTextureProgram = UAssetCache::Load<URenderPassProgram>("engine\\shaders\\CopyTexture.hlsl");
			loadedCopyTextureProgram = true;
		}

		static bool loadedDepthProgram = false;
		static eastl::shared_ptr<URenderPassProgram> depthProgram;
		if (!loadedDepthProgram)
		{
			depthProgram = UAssetCache::Load<URenderPassProgram>("engine\\shaders\\RenderDepth.hlsl");
			loadedDepthProgram = true;
		}

		SShaderResourceId target;
		switch(m_visualizeOption)
		{
		case EVisualizeOptions::LightAccumulation:
			// Already in the back buffer, so just return
			return;
		case EVisualizeOptions::Diffuse:
			target = m_gBufferShaderResources[1];
			break;
		case EVisualizeOptions::Specular:
			target = m_gBufferShaderResources[3];
			break;
		case EVisualizeOptions::Normals:
			target = m_gBufferShaderResources[2];
			break;
		case EVisualizeOptions::Depth:
			target = m_gBufferShaderResources[4];
			break;
		default: return;
		}

		if (m_visualizeOption == EVisualizeOptions::Depth)
		{
			depthProgram->SetProgramActive(g_graphicsDriver, 0);
		}
		else
		{
			copyTextureProgram->SetProgramActive(g_graphicsDriver, 0);
		}

		auto& renderTarget = m_gBufferPassDescriptor.m_renderTargets[AsIntegral(ERenderTargetSlot::LightingBuffer)];
		g_graphicsDriver.SetRenderTargets(&renderTarget, 1, nullptr);
		g_graphicsDriver.SetPixelShaderResource(target, ETextureSlot::DiffuseBuffer);

		DrawFullscreenQuad();
	}

	ProgramId_t URenderer::DetermineProgramId(const SDrawItem& inTargetDrawItem) const
	{
		ProgramId_t outputProgramId = 0;

		for (const auto& currentTextureSlot : inTargetDrawItem.m_shaderResources)
		{
			switch (currentTextureSlot.first)
			{
			case ETextureSlot::DiffuseMap: // Do you have a diffuse map?
				outputProgramId |= static_cast<ProgramId_t>(UProgramPermutor::EProgramIdMask::EProgramIdMask_Diffuse);
				break;
			case ETextureSlot::SpecularMap: // Do you have a specular map?
				outputProgramId |= static_cast<ProgramId_t>(UProgramPermutor::EProgramIdMask::EProgramIdMask_Specular);
				break;
			case ETextureSlot::EmissiveMap: // Do you have a emissive map?
				outputProgramId |= static_cast<ProgramId_t>(UProgramPermutor::EProgramIdMask::EProgramIdMask_Emissive);
				break;
			}
		}

		return outputProgramId;
	}

	SShaderResourceId URenderer::CreateTextureFromFile(const eastl::string& inPath, uint64_t& outWidth, uint64_t& outHeight) const
	{
		return g_graphicsDriver.CreateTextureFromFile(inPath, outWidth, outHeight);
	}

	void URenderer::SetFullScreen(bool inIsFullscreen) const
	{
		g_graphicsDriver.SetFullScreen(inIsFullscreen);
	}

	void URenderer::UpdateCameraConstants(const SCameraInstance& inCameraInstance)
	{
		m_perFrameConstants.m_cameraViewMatrix = inCameraInstance.m_viewMatrix;
		m_perFrameConstants.m_cameraProjectionMatrix = inCameraInstance.m_projectionMatrix;
		m_perFrameConstants.m_cameraViewProjectionMatrix = inCameraInstance.m_viewProjectionMatrix;
		m_perFrameConstants.m_cameraInverseProjectionMatrix = inCameraInstance.m_projectionMatrix.Invert();
		m_perFrameConstants.m_cameraNearPlane = inCameraInstance.m_nearPlaneDistance;
		m_perFrameConstants.m_cameraFarPlane = inCameraInstance.m_farPlaneDistance;
	}

	void URenderer::SetWorldAmbientColor(Color inColor)
	{
		// Convert from sRGB space to linear color space.
		// This is then converted back to sRGB space when rendering to the back buffer
		// and then back to linear by the monitor...
		m_perSceneConstants.m_ambientColor.x = powf(inColor.x, 2.2f);
		m_perSceneConstants.m_ambientColor.y = powf(inColor.y, 2.2f);
		m_perSceneConstants.m_ambientColor.z = powf(inColor.z, 2.2f);
		m_perSceneConstants.m_ambientColor.w = 1.0f;

		g_graphicsDriver.UpdateBuffer(EConstantBufferSlot::PerScene, &m_perSceneConstants, sizeof(m_perSceneConstants));
	}

	void URenderer::SetBackBufferClearColor(Color inColor)
	{
		m_clearColor.x = powf(inColor.x, 2.2f);
		m_clearColor.y = powf(inColor.y, 2.2f);
		m_clearColor.z = powf(inColor.z, 2.2f);
		m_clearColor.w = 1.0f;
	}

	class UGraphicsDriver& URenderer::GetGraphicsDriver()
	{
		return g_graphicsDriver;
	}
}
