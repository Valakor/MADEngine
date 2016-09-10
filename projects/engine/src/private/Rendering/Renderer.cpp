#include "Rendering/Renderer.h"

#include "Core/GameWindow.h"
#include "Misc/Assert.h"
#include "Misc/Logging.h"
#include "Misc/utf8conv.h"
#include "Rendering/GraphicsDriver.h"

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

		LOG(LogRenderer, Log, "Renderer initialization successful\n");
		return true;
	}

	void URenderer::Shutdown()
	{
		g_graphicsDriver.Shutdown();
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

	void URenderer::BeginFrame()
	{
		static const FLOAT clearColor[] = { 0.392f, 0.584f, 0.929f, 1.0f };
		g_graphicsDriver.ClearBackBuffer(clearColor);
	}

	void URenderer::Draw()
	{
		
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
