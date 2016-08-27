#include "Rendering/Renderer.h"

namespace MAD
{
	DECLARE_LOG_CATEGORY(LogRenderer);

	URenderer::URenderer()
	{ }

	bool URenderer::Init()
	{
		LOG(LogRenderer, Log, "Renderer initialization begin...\n");

		LOG(LogRenderer, Log, "Renderer initialization successful\n");
		return true;
	}

	void URenderer::Shutdown()
	{

	}

	void URenderer::OnScreenSizeChanged()
	{

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

	}

	void URenderer::Draw()
	{
		
	}

	void URenderer::EndFrame()
	{
		
	}
}
