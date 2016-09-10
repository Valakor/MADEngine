#include "Rendering/Renderer.h"
#include "Rendering/RenderPassProgram.h"

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

		m_gBufferPassDescriptor.m_renderTargets.emplace_back(g_graphicsDriver.GetBackBufferRenderTarget());
		m_gBufferPassDescriptor.m_renderTargets.emplace_back(g_graphicsDriver.CreateRenderTarget(clientSize.x, clientSize.y, DXGI_FORMAT_B8G8R8A8_UNORM));
		m_gBufferPassDescriptor.m_renderTargets.emplace_back(g_graphicsDriver.CreateRenderTarget(clientSize.x, clientSize.y, DXGI_FORMAT_B8G8R8A8_UNORM));

		m_gBufferPassDescriptor.m_depthStencilView = g_graphicsDriver.CreateDepthStencil(clientSize.x, clientSize.y);
		m_gBufferPassDescriptor.m_depthStencilState = g_graphicsDriver.CreateDepthStencilState(true, D3D11_COMPARISON_LESS);
		m_gBufferPassDescriptor.m_rasterizerState = g_graphicsDriver.CreateRasterizerState(D3D11_FILL_SOLID, D3D11_CULL_BACK);

		m_gBufferPassDescriptor.m_renderPassProgram = UAssetCache::Load<URenderPassProgram>(inGBufferProgramPath);
	}

	void URenderer::InitializeLightingPass(const eastl::string& inLightingPassProgramPath)
	{
		(void)inLightingPassProgramPath;
	}

	void URenderer::BeginFrame()
	{
		static const FLOAT clearColor[] = { 0.392f, 0.584f, 0.929f, 1.0f };
		g_graphicsDriver.ClearBackBuffer(clearColor);
	}

	void URenderer::Draw()
	{
		m_gBufferPassDescriptor.ApplyPassState(g_graphicsDriver);

		// Go through each draw item and bind input assembly data
		for (const SDrawItem& currentDrawItem : m_queuedDrawItems)
		{
			// Each individual DrawItem should issue it's own draw call
			currentDrawItem.Draw(g_graphicsDriver);
		}

		m_queuedDrawItems.clear();
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

	class UGraphicsDriver& URenderer::GetGraphicsDriver()
	{
		return g_graphicsDriver;
	}


}
