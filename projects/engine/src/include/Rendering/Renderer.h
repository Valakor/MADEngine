#pragma once

#include <EASTL/shared_ptr.h>
#include <EASTL/string.h>
#include <EASTl/vector.h>

#include "Rendering/GraphicsDriverTypes.h"
#include "Rendering/RenderingCommon.h"
#include "Rendering/RenderPassDescriptor.h"
#include "Rendering/DrawItem.h"

namespace MAD
{
	class UGameWindow;

	class URenderer
	{
	public:
		URenderer();

		bool Init(UGameWindow& inWindow);
		void Shutdown();

		void QueueDrawItem(const SDrawItem& inDrawItem);

		void Frame(float framePercent);

		void OnScreenSizeChanged();

		void SetFullScreen(bool inIsFullscreen) const;

		void UpdateCameraConstants(const struct SCameraInstance& inCameraInstance);

		class UGraphicsDriver& GetGraphicsDriver();

		// Don't use this API to make a Texture, use UAssetCache<UTexture> instead
		SShaderResourceId URenderer::CreateTextureFromFile(const eastl::string& inPath, uint64_t& outWidth, uint64_t& outHeight) const;
	private:
		// TODO: Eventually be able to initiliaze/load them from file
		void InitializeGBufferPass(const eastl::string& inGBufferPassProgramPath);
		void InitializeLightingPass(const eastl::string& inLightingPassProgramPath);

		void BeginFrame();
		void Draw();
		void EndFrame();
	private:
		UGameWindow* m_window;
		SRenderTargetId m_backBuffer;
		SPerFrameConstants m_perFrameConstants;
		SRenderPassDescriptor m_gBufferPassDescriptor;
		SRenderPassDescriptor m_lightingPassDescriptor;
		eastl::vector<SDrawItem> m_queuedDrawItems;
	};
}
