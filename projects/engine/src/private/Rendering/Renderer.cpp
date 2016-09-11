#include "Rendering/Renderer.h"
#include "Rendering/RenderPassProgram.h"
#include "Rendering/CameraInstance.h"

#include "Core/GameWindow.h"
#include "Misc/AssetCache.h"
#include "Misc/Assert.h"
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

	URenderer::URenderer(): m_window(nullptr) { }

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
		g_graphicsDriver.SetViewport(0, 0, clientSize.x, clientSize.y);

		InitializeGBufferPass("engine\\shaders\\GBuffer.hlsl");
		InitializeLightingPass("engine\\shaders\\DeferredLighting.hlsl");

		g_graphicsDriver.SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

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

	void URenderer::OnScreenSizeChanged()
	{
		auto newSize = m_window->GetClientSize();
		LOG(LogRenderer, Log, "OnScreenSizeChanged: { %i, %i }\n", newSize.x, newSize.y);

		g_graphicsDriver.OnScreenSizeChanged();

		InitializeGBufferPass("engine\\shaders\\GBuffer.hlsl");
		InitializeLightingPass("engine\\shaders\\DeferredLighting.hlsl");

		auto clientSize = m_window->GetClientSize();
		g_graphicsDriver.SetViewport(0, 0, clientSize.x, clientSize.y);
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

		g_graphicsDriver.DestroyDepthStencil(m_gBufferPassDescriptor.m_depthStencilView);
		m_gBufferPassDescriptor.m_depthStencilView = g_graphicsDriver.CreateDepthStencil(clientSize.x, clientSize.y);
		m_gBufferPassDescriptor.m_depthStencilState = g_graphicsDriver.CreateDepthStencilState(true, D3D11_COMPARISON_LESS);

		for (unsigned i = 1; i < m_gBufferPassDescriptor.m_renderTargets.size(); ++i)
		{
			g_graphicsDriver.DestroyRenderTarget(m_gBufferPassDescriptor.m_renderTargets[i]);
		}

		// TODO Give the SRV's for each RenderTarget to the LightingPassDescriptor somehow?
		m_gBufferPassDescriptor.m_renderTargets.resize(AsIntegral(ERenderTargetSlot::MAX));
		m_gBufferPassDescriptor.m_renderTargets[AsIntegral(ERenderTargetSlot::BackBuffer)] = g_graphicsDriver.GetBackBufferRenderTarget();
		m_gBufferPassDescriptor.m_renderTargets[AsIntegral(ERenderTargetSlot::DiffuseBuffer)] = g_graphicsDriver.CreateRenderTarget(clientSize.x, clientSize.y, DXGI_FORMAT_R8G8B8A8_UNORM);
		m_gBufferPassDescriptor.m_renderTargets[AsIntegral(ERenderTargetSlot::NormalBuffer)] = g_graphicsDriver.CreateRenderTarget(clientSize.x, clientSize.y, DXGI_FORMAT_R16G16_FLOAT);
		m_gBufferPassDescriptor.m_renderTargets[AsIntegral(ERenderTargetSlot::SpecularBuffer)] = g_graphicsDriver.CreateRenderTarget(clientSize.x, clientSize.y, DXGI_FORMAT_R8G8B8A8_UNORM);

		m_gBufferPassDescriptor.m_rasterizerState = g_graphicsDriver.CreateRasterizerState(D3D11_FILL_SOLID, D3D11_CULL_NONE);

		m_gBufferPassDescriptor.m_renderPassProgram = UAssetCache::Load<URenderPassProgram>(inGBufferProgramPath);

		m_gBufferPassDescriptor.m_blendState = g_graphicsDriver.CreateBlendState(false);
	}

	void URenderer::InitializeLightingPass(const eastl::string& inLightingPassProgramPath)
	{
		(void)inLightingPassProgramPath;
	}

	void URenderer::BindPerFrameConstants()
	{
		// Update the per frame constant buffer
		g_graphicsDriver.UpdateBuffer(EConstantBufferSlot::PerFrame, &m_perFrameConstants, sizeof(m_perFrameConstants));
	}

	void URenderer::BeginFrame()
	{
		g_graphicsDriver.ClearBackBuffer(m_clearColor);
	}

	void URenderer::Draw()
	{
		// Bind per-frame constants
		BindPerFrameConstants();

		m_gBufferPassDescriptor.ApplyPassState(g_graphicsDriver);

		// Go through each draw item and bind input assembly data
		for (const SDrawItem& currentDrawItem : m_queuedDrawItems)
		{
			// Each individual DrawItem should issue it's own draw call
			currentDrawItem.Draw(g_graphicsDriver, true);
		}
	}

	void URenderer::EndFrame()
	{
		g_graphicsDriver.Present();
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
