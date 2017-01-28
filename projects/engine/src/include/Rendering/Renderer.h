#pragma once

#include <EASTL/string.h>
#include <EASTl/vector.h>
#include <EASTL/array.h>
#include <EASTL/unique_ptr.h>

#include "Rendering/GraphicsDriverTypes.h"
#include "Rendering/RenderingCommon.h"
#include "Rendering/RenderPassDescriptor.h"
#include "Rendering/RenderPassProgram.h"
#include "Rendering/DrawItem.h"
#include "Rendering/CameraInstance.h"
#include "Rendering/DepthTextureCube.h"

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

	struct SDebugHandle
	{
		double m_initialGameTime;
		float m_duration;
		SDrawItem m_debugDrawItem;
	};

	class URenderer
	{
	public:
		URenderer();

		bool Init(UGameWindow& inWindow);
		void Shutdown();

		void QueueDrawItem(const SDrawItem& inDrawItem);
		void QueueDebugDrawItem(const SDrawItem& inDebugDrawItem, float inDuration = 0.0f);
		void QueueDirectionalLight(size_t inID, const SGPUDirectionalLight& inDirectionalLight);
		void QueuePointLight(size_t inID, const SGPUPointLight& inPointLight);

		void DrawDebugLine(const Vector3& inWSStart, const Vector3& inWSEnd, float inDuration, const Color& inLineColor = Color(1.0, 1.0, 1.0, 1.0));

		void ClearRenderItems();

		void Frame(float framePercent);

		void OnScreenSizeChanged();

		void SetFullScreen(bool inIsFullscreen) const;

		void UpdateCameraConstants(const struct SCameraInstance& inCameraInstance);
		void CalculateCameraConstants(float inFramePercent);
		void SetWorldAmbientColor(Color inColor);
		void SetBackBufferClearColor(Color inColor);

		class UGraphicsDriver& GetGraphicsDriver();
		RasterizerStatePtr_t GetRasterizerState(D3D11_FILL_MODE inFillMode, D3D11_CULL_MODE inCullMode) const;

		void SetGBufferVisualizeOption(EVisualizeOptions inOption) { m_visualizeOption = inOption; }
		void ToggleDebugLayerEnabled() { m_isDebugLayerEnabled = !m_isDebugLayerEnabled; }
	private:
		// TODO: Eventually be able to initiliaze/load them from file
		void InitializeGBufferPass(const eastl::string& inGBufferPassProgramPath);
		void InitializeDirectionalLightingPass(const eastl::string& inLightingPassProgramPath);
		void InitializePointLightingPass(const eastl::string& inLightingPassProgramPath);
		void InitializeDebugPass(const eastl::string& inProgramPath);

		void InitializeDirectionalShadowMappingPass(const eastl::string& inProgramPath);
		void InitializePointLightShadowMappingPass(const eastl::string& inProgramPath);

		void InitializeDebugGrid(uint8_t inGridDimension);

		void PopulatePointShadowVPMatrices(const Vector3& inWSLightPos, TextureCubeVPArray_t& inOutVPArray);
		void PopulateDebugLineVertices(const Vector3& inMSStart, const Vector3& inMSEnd, const Color& inLineColor, eastl::vector<UVertexArray>& inOutLineVertexData);

		void BindPerFrameConstants();
		void SetViewport(LONG inWidth, LONG inHeight);

		void BeginFrame();
		void Draw(float inFramePercent);
		void EndFrame();

		void ClearExpiredDebugDrawItems();

		void DrawDirectionalLighting(float inFramePercent);
		void DrawPointLighting(float inFramePercent);
		void DrawDebugPrimitives(float inFramePerecent);

		void DoVisualizeGBuffer();

		ProgramId_t DetermineProgramId(const SDrawItem& inTargetDrawItem) const;
	private:
		uint32_t m_frame;

		UGameWindow* m_window;
		RenderTargetPtr_t m_backBuffer;
		SPerSceneConstants m_perSceneConstants;
		SPerFrameConstants m_perFrameConstants;
		Color m_clearColor;
		bool m_isDebugLayerEnabled;

		// Double-buffer draw items for current and past frame for state interpolation
		int m_currentStateIndex;
		SCameraInstance m_camera[2];

		eastl::vector<SDebugHandle> m_debugDrawItems;

		eastl::hash_map<size_t, SDrawItem> m_queuedDrawItems[2];
		eastl::hash_map<size_t, SGPUDirectionalLight> m_queuedDirLights[2];
		eastl::hash_map<size_t, SGPUPointLight> m_queuedPointLights[2];
		
		SRenderPassDescriptor m_gBufferPassDescriptor;
		SRenderPassDescriptor m_dirLightingPassDescriptor;
		SRenderPassDescriptor m_pointLightingPassDescriptor;
		SRenderPassDescriptor m_dirShadowMappingPassDescriptor;
		SRenderPassDescriptor m_pointShadowMappingPassDescriptor;
		SRenderPassDescriptor m_debugPassDescriptor;

		ShaderResourcePtr_t m_shadowMapSRV;

		eastl::vector<ShaderResourcePtr_t> m_gBufferShaderResources;
		eastl::unique_ptr<UDepthTextureCube> m_depthTextureCube;

		EVisualizeOptions m_visualizeOption;
	};
}
