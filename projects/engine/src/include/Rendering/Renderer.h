#pragma once

#include <EASTL/shared_ptr.h>
#include <EASTL/string.h>

#include "Rendering/GraphicsDriverTypes.h"

namespace MAD
{
	class UGameWindow;

	class URenderer
	{
	public:
		URenderer();

		bool Init(UGameWindow& inWindow);
		void Shutdown();

		void Frame(float framePercent);

		void OnScreenSizeChanged();

		void SetFullScreen(bool inIsFullscreen) const;

		class UGraphicsDriver& GetGraphicsDriver();

		// Don't use this API to make a Texture, use UAssetCache<UTexture> instead
		SShaderResourceId URenderer::CreateTextureFromFile(const eastl::string& inPath, uint64_t& outWidth, uint64_t& outHeight) const;

	private:
		void BeginFrame();
		void Draw();
		void EndFrame();

		UGameWindow* m_window;
		SRenderTargetId m_backBuffer;
	};
}
