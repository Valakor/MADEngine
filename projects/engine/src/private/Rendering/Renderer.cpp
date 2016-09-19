#include "Rendering/Renderer.h"
#include "Rendering/RenderPassProgram.h"
#include "Rendering/CameraInstance.h"

#include "Core/GameWindow.h"
#include "Misc/AssetCache.h"
#include "Misc/Logging.h"
#include "Misc/utf8conv.h"
#include "Rendering/GraphicsDriver.h"

#include <EASTL/array.h>

namespace MAD
{
	DECLARE_LOG_CATEGORY(LogRenderer);

	namespace
	{
		UGraphicsDriver g_graphicsDriver;
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

		m_gBufferShaderResources.resize(4);
		g_graphicsDriver.SetPixelShaderResource(SShaderResourceId::Invalid, ETextureSlot::DiffuseBuffer);
		g_graphicsDriver.SetPixelShaderResource(SShaderResourceId::Invalid, ETextureSlot::NormalBuffer);
		g_graphicsDriver.SetPixelShaderResource(SShaderResourceId::Invalid, ETextureSlot::SpecularBuffer);
		g_graphicsDriver.SetPixelShaderResource(SShaderResourceId::Invalid, ETextureSlot::DepthBuffer);
		for (unsigned i = 0; i < m_gBufferShaderResources.size(); ++i)
		{
			g_graphicsDriver.DestroyShaderResource(m_gBufferShaderResources[i]);
		}

		g_graphicsDriver.DestroyDepthStencil(m_gBufferPassDescriptor.m_depthStencilView);
		SShaderResourceId& depthBufferSRV = m_gBufferShaderResources[AsIntegral(ETextureSlot::DepthBuffer) - AsIntegral(ETextureSlot::DiffuseBuffer)];
		m_gBufferPassDescriptor.m_depthStencilView = g_graphicsDriver.CreateDepthStencil(clientSize.x, clientSize.y, &depthBufferSRV);
		m_gBufferPassDescriptor.m_depthStencilState = g_graphicsDriver.CreateDepthStencilState(true, D3D11_COMPARISON_LESS);

		for (unsigned i = 1; i < m_gBufferPassDescriptor.m_renderTargets.size(); ++i)
		{
			g_graphicsDriver.DestroyRenderTarget(m_gBufferPassDescriptor.m_renderTargets[i]);
		}
		
		SShaderResourceId& diffuseBufferSRV = m_gBufferShaderResources[AsIntegral(ETextureSlot::DiffuseBuffer) - AsIntegral(ETextureSlot::DiffuseBuffer)];
		SShaderResourceId& normalBufferSRV = m_gBufferShaderResources[AsIntegral(ETextureSlot::NormalBuffer) - AsIntegral(ETextureSlot::DiffuseBuffer)];
		SShaderResourceId& specularBufferSRV = m_gBufferShaderResources[AsIntegral(ETextureSlot::SpecularBuffer) - AsIntegral(ETextureSlot::DiffuseBuffer)];

		m_gBufferPassDescriptor.m_renderTargets.resize(AsIntegral(ERenderTargetSlot::MAX));
		m_gBufferPassDescriptor.m_renderTargets[AsIntegral(ERenderTargetSlot::BackBuffer)] = g_graphicsDriver.GetBackBufferRenderTarget();
		m_gBufferPassDescriptor.m_renderTargets[AsIntegral(ERenderTargetSlot::DiffuseBuffer)] = g_graphicsDriver.CreateRenderTarget(clientSize.x, clientSize.y, DXGI_FORMAT_R8G8B8A8_UNORM, &diffuseBufferSRV);
		m_gBufferPassDescriptor.m_renderTargets[AsIntegral(ERenderTargetSlot::NormalBuffer)] = g_graphicsDriver.CreateRenderTarget(clientSize.x, clientSize.y, DXGI_FORMAT_R16G16_UNORM, &normalBufferSRV);
		m_gBufferPassDescriptor.m_renderTargets[AsIntegral(ERenderTargetSlot::SpecularBuffer)] = g_graphicsDriver.CreateRenderTarget(clientSize.x, clientSize.y, DXGI_FORMAT_R8G8B8A8_UNORM, &specularBufferSRV);
#ifdef _DEBUG
		g_graphicsDriver.SetDebugName_RenderTarget(m_gBufferPassDescriptor.m_renderTargets[AsIntegral(ERenderTargetSlot::BackBuffer)], "Light Accumulation Buffer");
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

		m_dirLightingPassDescriptor.m_renderTargets.push_back(g_graphicsDriver.GetBackBufferRenderTarget());

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
		g_graphicsDriver.ClearBackBuffer(m_clearColor);
		g_graphicsDriver.ClearDepthStencil(m_gBufferPassDescriptor.m_depthStencilView, true, 1.0f);
		for (unsigned i = 1; i < m_gBufferPassDescriptor.m_renderTargets.size(); ++i)
		{
			static const float zero[] = { 0.0f, 0.0f, 0.0f, 0.0f };
			auto renderTarget = m_gBufferPassDescriptor.m_renderTargets[i];
			g_graphicsDriver.ClearRenderTarget(renderTarget, zero);
		}

		g_graphicsDriver.SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	void URenderer::Draw()
	{
		// Bind per-frame constants
		BindPerFrameConstants();

		g_graphicsDriver.SetPixelShaderResource(SShaderResourceId::Invalid, ETextureSlot::DiffuseBuffer);
		g_graphicsDriver.SetPixelShaderResource(SShaderResourceId::Invalid, ETextureSlot::NormalBuffer);
		g_graphicsDriver.SetPixelShaderResource(SShaderResourceId::Invalid, ETextureSlot::SpecularBuffer);
		g_graphicsDriver.SetPixelShaderResource(SShaderResourceId::Invalid, ETextureSlot::DepthBuffer);

		m_gBufferPassDescriptor.ApplyPassState(g_graphicsDriver);

		// Go through each draw item and bind input assembly data
		for (const SDrawItem& currentDrawItem : m_queuedDrawItems)
		{
			// Each individual DrawItem should issue it's own draw call
			currentDrawItem.Draw(g_graphicsDriver, true);
		}

		if (m_visualizeOption != EVisualizeOptions::None)
		{
			DoVisualizeGBuffer();
			return;
		}

		m_dirLightingPassDescriptor.ApplyPassState(g_graphicsDriver);
		g_graphicsDriver.SetPixelShaderResource(m_gBufferShaderResources[0], ETextureSlot::DiffuseBuffer);
		g_graphicsDriver.SetPixelShaderResource(m_gBufferShaderResources[1], ETextureSlot::NormalBuffer);
		g_graphicsDriver.SetPixelShaderResource(m_gBufferShaderResources[2], ETextureSlot::SpecularBuffer);
		g_graphicsDriver.SetPixelShaderResource(m_gBufferShaderResources[3], ETextureSlot::DepthBuffer);
		g_graphicsDriver.SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		for (const SGPUDirectionalLight& currentDirLight : m_queuedDirLights)
		{
			g_graphicsDriver.UpdateBuffer(EConstantBufferSlot::PerDirectionalLight, &currentDirLight, sizeof(SGPUDirectionalLight));
			g_graphicsDriver.Draw(4, 0);
		}
	}

	void URenderer::EndFrame()
	{
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

		SShaderResourceId target;

		auto backBuffer = g_graphicsDriver.GetBackBufferRenderTarget();
		g_graphicsDriver.SetRenderTargets(&backBuffer, 1, nullptr);
		copyTextureProgram->SetProgramActive(g_graphicsDriver);

		switch(m_visualizeOption)
		{
		case EVisualizeOptions::LightAccumulation:
			// Already in the back buffer, so just return
			return;
		case EVisualizeOptions::Diffuse:
			target = m_gBufferShaderResources[0];
			break;
		case EVisualizeOptions::Specular:
			target = m_gBufferShaderResources[2];
			break;
		case EVisualizeOptions::Normals:
			target = m_gBufferShaderResources[1];
			break;
		case EVisualizeOptions::Depth:
			target = m_gBufferShaderResources[3];
			break;
		default: return;
		}

		g_graphicsDriver.SetPixelShaderResource(target, ETextureSlot::DiffuseBuffer);
		g_graphicsDriver.SetRasterizerState(m_dirLightingPassDescriptor.m_rasterizerState);

		g_graphicsDriver.SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		g_graphicsDriver.Draw(4, 0);
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
	}

	void URenderer::SetWorldAmbientColor(DirectX::SimpleMath::Color inColor)
	{
		m_perSceneConstants.m_ambientColor = inColor;
		g_graphicsDriver.UpdateBuffer(EConstantBufferSlot::PerScene, &m_perSceneConstants, sizeof(m_perSceneConstants));
	}

	void URenderer::SetBackBufferClearColor(DirectX::SimpleMath::Color inColor)
	{
		m_clearColor = inColor;
	}

	class UGraphicsDriver& URenderer::GetGraphicsDriver()
	{
		return g_graphicsDriver;
	}
}
