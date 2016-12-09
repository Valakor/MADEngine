#pragma once

#include <EASTL/string.h>
#include <EASTl/vector.h>

#include "Rendering/GraphicsDriverTypes.h"
#include "Rendering/RenderingCommon.h"
#include "Rendering/RenderPassDescriptor.h"
#include "Rendering/RenderPassProgram.h"
#include "Rendering/DrawItem.h"
#include "Rendering/CameraInstance.h"

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
		void QueueDirectionalLight(size_t inID, const SGPUDirectionalLight& inDirectionalLight);
		void QueuePointLight(size_t inID, const SGPUPointLight& inPointLight);

		void ClearRenderItems();

		void Frame(float framePercent);

		void OnScreenSizeChanged();

		void SetFullScreen(bool inIsFullscreen) const;

		void UpdateCameraConstants(const struct SCameraInstance& inCameraInstance);
		void CalculateCameraConstants(float inFramePercent);
		void SetWorldAmbientColor(Color inColor);
		void SetBackBufferClearColor(Color inColor);

		class UGraphicsDriver& GetGraphicsDriver();
		SRasterizerStateId GetRasterizerState(D3D11_FILL_MODE inFillMode, D3D11_CULL_MODE inCullMode) const;

		void SetGBufferVisualizeOption(EVisualizeOptions inOption) { m_visualizeOption = inOption; }

	private:
		// TODO: Eventually be able to initiliaze/load them from file
		void InitializeGBufferPass(const eastl::string& inGBufferPassProgramPath);
		void InitializeDirectionalLightingPass(const eastl::string& inLightingPassProgramPath);
		void InitializeDirectionalShadowMappingPass(const eastl::string& inProgramPath);
		void InitializePointLightingPass(const eastl::string& inLightingPassProgramPath);

		void BindPerFrameConstants();
		void SetViewport(LONG inWidth, LONG inHeight);

		void BeginFrame();
		void Draw(float inFramePercent);
		void EndFrame();

		void DoVisualizeGBuffer();

		ProgramId_t DetermineProgramId(const SDrawItem& inTargetDrawItem) const;
	private:
		uint32_t m_frame;

		UGameWindow* m_window;
		SRenderTargetId m_backBuffer;
		SPerSceneConstants m_perSceneConstants;
		SPerFrameConstants m_perFrameConstants;
		Color m_clearColor;

		// Double-buffer draw items for current and past frame for state interpolation
		int m_currentStateIndex;
		SCameraInstance m_camera[2];
		eastl::hash_map<size_t, SDrawItem> m_queuedDrawItems[2];
		eastl::hash_map<size_t, SGPUDirectionalLight> m_queuedDirLights[2];
		eastl::hash_map<size_t, SGPUPointLight> m_queuedPointLights[2];
		
		SRenderPassDescriptor m_gBufferPassDescriptor;
		SRenderPassDescriptor m_dirLightingPassDescriptor;
		SRenderPassDescriptor m_dirShadowMappingPassDescriptor;
		SRenderPassDescriptor m_pointLightingPassDescriptor;

		SShaderResourceId m_shadowMapSRV;

		eastl::vector<SShaderResourceId> m_gBufferShaderResources;

		EVisualizeOptions m_visualizeOption;
	};
}
