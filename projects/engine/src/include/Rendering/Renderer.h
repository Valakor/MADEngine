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

	enum class EVisualizeOptions
	{
		None,
		LightAccumulation,
		Diffuse,
		Specular,
		Normals,
		Depth
	};

	class URenderer
	{
	public:
		URenderer();

		bool Init(UGameWindow& inWindow);
		void Shutdown();

		void QueueDrawItem(const SDrawItem& inDrawItem);
		void QueueDirectionLight(const SGPUDirectionalLight& inDirectionalLight);

		void ClearRenderItems() { m_queuedDrawItems.clear(); m_queuedDirLights.clear(); }

		void Frame(float framePercent);

		void OnScreenSizeChanged();

		void SetFullScreen(bool inIsFullscreen) const;

		const SPerFrameConstants& GetCameraConstants() const { return m_perFrameConstants; }
		void UpdateCameraConstants(const struct SCameraInstance& inCameraInstance);
		void SetWorldAmbientColor(DirectX::SimpleMath::Color inColor);
		void SetBackBufferClearColor(DirectX::SimpleMath::Color inColor);

		class UGraphicsDriver& GetGraphicsDriver();

		// Don't use this API to make a Texture, use UAssetCache<UTexture> instead
		SShaderResourceId URenderer::CreateTextureFromFile(const eastl::string& inPath, uint64_t& outWidth, uint64_t& outHeight) const;

		void SetGBufferVisualizeOption(EVisualizeOptions inOption) { m_visualizeOption = inOption; }

	private:
		// TODO: Eventually be able to initiliaze/load them from file
		void InitializeGBufferPass(const eastl::string& inGBufferPassProgramPath);
		void InitializeDirectionalLightingPass(const eastl::string& inLightingPassProgramPath);

		void BindPerFrameConstants();
		void SetViewport(LONG inWidth, LONG inHeight);

		void BeginFrame();
		void Draw();
		void EndFrame();

		void DoVisualizeGBuffer();

	private:
		UGameWindow* m_window;
		SRenderTargetId m_backBuffer;
		SPerSceneConstants m_perSceneConstants;
		SPerFrameConstants m_perFrameConstants;
		DirectX::SimpleMath::Color m_clearColor;

		eastl::vector<SDrawItem> m_queuedDrawItems;
		eastl::vector<SGPUDirectionalLight> m_queuedDirLights;
		
		SRenderPassDescriptor m_gBufferPassDescriptor;
		SRenderPassDescriptor m_dirLightingPassDescriptor;

		eastl::vector<SShaderResourceId> m_gBufferShaderResources;

		EVisualizeOptions m_visualizeOption;
	};
}
