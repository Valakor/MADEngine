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
#include "Rendering/ColorTextureCube.h"
#include "Rendering/TextBatchRenderer.h"
#include "Rendering/SkySphere.h"

#include "Rendering/ParticleSystem/ParticleSystemManager.h"

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

		void QueueDynamicItem(const SDrawItem& inDrawItem);
		void QueueStaticItem(const SDrawItem& inDrawItem);
		void QueueReflectionProbeItem(const SDrawItem& inDrawItem);
		void QueueDebugItem(const SDrawItem& inDebugDrawItem, float inDuration = 0.0f);
		void QueueDirectionalLight(size_t inID, const SGPUDirectionalLight& inDirectionalLight);
		void QueuePointLight(size_t inID, const SGPUPointLight& inPointLight);

		void DrawDebugLine(const Vector3& inWSStart, const Vector3& inWSEnd, float inDuration, const Color& inLineColor = Color(1.0, 1.0, 1.0, 1.0));
		void DrawOnScreenText(const eastl::string& inSourceString, float inScreenX, float inScreenY);

		class UParticleSystem* SpawnParticleSystem(const SParticleSystemSpawnParams& inSpawnParams, const eastl::vector<SParticleEmitterSpawnParams>& inEmitterParams);

		void ClearRenderItems();

		void Frame(float inFramePercent, float inFrameTime);

		void OnScreenSizeChanged();

		void SetFullScreen(bool inIsFullscreen) const;

		void UpdateCameraConstants(const struct SCameraInstance& inCameraInstance);
		void CalculateCameraConstants(float inFramePercent);
		void SetWorldAmbientColor(const Color& inColor);
		void SetBackBufferClearColor(const Color& inColor);

		// TODO Refactor the Renderer to allow for a better interface than this? Separate graphics sub-systems will need to be able to
		// retrieve pass information (such as the g-buffer pass information)
		const SRenderPassDescriptor& GetGBufferPassDescriptor() const { return m_gBufferPassDescriptor; }
		class UGraphicsDriver& GetGraphicsDriver();
		const SPerFrameConstants& GetPerFrameConstants() const { return m_perFrameConstants; }

		RasterizerStatePtr_t GetRasterizerState(EFillMode inFillMode, ECullMode inCullMode) const;
		void SetGBufferVisualizeOption(EVisualizeOptions inOption) { m_visualizeOption = inOption; }
		void ToggleDebugLayerEnabled() { m_isDebugLayerEnabled = !m_isDebugLayerEnabled; }
		void ToggleTextBatching();

		POINT GetScreenSize() const;
	private:
		// TODO: Eventually be able to initiliaze/load them from file
		void InitializeRenderPasses();

		void InitializeReflectionPass(const eastl::string& inReflectionPassProgramPath);
		void InitializeGBufferPass(const eastl::string& inGBufferPassProgramPath);
		void InitializeDirectionalLightingPass(const eastl::string& inLightingPassProgramPath);
		void InitializePointLightingPass(const eastl::string& inLightingPassProgramPath);
		void InitializeDebugPass(const eastl::string& inProgramPath);
		void InitializeTextRenderPass(const eastl::string& inProgramPath);

		void InitializeDirectionalShadowMappingPass(const eastl::string& inProgramPath);
		void InitializePointLightShadowMappingPass(const eastl::string& inProgramPath);

		void InitializeDebugGrid(uint8_t inGridDimension);

		void GenerateViewProjectionMatrices(const Vector3& inWSPos, CubeTransformArray_t& inOutVPArray);
		void GenerateViewMatrices(const Vector3& inWSPos, CubeTransformArray_t& inOutViewsArray, Matrix& inOutProjectionMatrix);
		void GenerateDebugLineVertices(const Vector3& inMSStart, const Vector3& inMSEnd, const Color& inLineColor, eastl::vector<UVertexArray>& inOutLineVertexData);

		void BindPerFrameConstants();
		void SetViewport(LONG inWidth, LONG inHeight);
		void SetViewport(const SGraphicsViewport& inViewport);

		void BeginFrame();
		void Draw(float inFramePercent, float inFrameTime);
		void EndFrame();

		void ClearExpiredDebugDrawItems();

		void DrawGBuffer(float inFramePercent);
		void DrawDirectionalLighting(float inFramePercent);
		void DrawPointLighting(float inFramePercent);
		void DrawDebugPrimitives(float inFramePerecent);

		void ProcessReflectionProbes(float inFramePercent);
		void DoVisualizeGBuffer();

		ProgramId_t DetermineProgramId(const SDrawItem& inTargetDrawItem) const;
	private:
		uint32_t m_frame;

		UGameWindow* m_window;
		RenderTargetPtr_t m_backBuffer;
		SPerSceneConstants m_perSceneConstants;
		SPerFrameConstants m_perFrameConstants;
		SGraphicsViewport m_screenViewport;
		Color m_clearColor;
		bool m_isDebugLayerEnabled;

		// Double-buffer draw items for current and past frame for state interpolation
		int m_currentStateIndex;
		SCameraInstance m_camera[2];

		eastl::vector<SDebugHandle> m_debugDrawItems;

		eastl::hash_map<size_t, SDrawItem> m_staticDrawItems; // Static draw items (don't need double buffer since we don't need interpolation for static objects)
		eastl::hash_map<size_t, SDrawItem> m_reflectionProbeDrawItems;
		eastl::hash_map<size_t, SDrawItem> m_dynamicDrawItems[2]; // Dynamic draw items


		eastl::hash_map<size_t, SGPUDirectionalLight> m_queuedDirLights[2];
		eastl::hash_map<size_t, SGPUPointLight> m_queuedPointLights[2];
		
		SRenderPassDescriptor m_reflectionPassDescriptor;
		SRenderPassDescriptor m_gBufferPassDescriptor;
		SRenderPassDescriptor m_dirLightingPassDescriptor;
		SRenderPassDescriptor m_pointLightingPassDescriptor;
		SRenderPassDescriptor m_dirShadowMappingPassDescriptor;
		SRenderPassDescriptor m_pointShadowMappingPassDescriptor;
		SRenderPassDescriptor m_debugPassDescriptor;
		SRenderPassDescriptor m_textRenderPassDescriptor;

		ShaderResourcePtr_t m_shadowMapSRV;

		eastl::vector<ShaderResourcePtr_t> m_gBufferShaderResources;
		eastl::unique_ptr<UDepthTextureCube> m_depthTextureCube;

		UTextBatchRenderer m_textBatchRenderer;
		UParticleSystemManager m_particleSystemManager; // Use defaults for now
		UColorTextureCube m_globalEnvironmentMap;
		// TEMP-------
		UColorTextureCube m_dynamicEnvironmentMap; // For now, just render everything from some specified texture cube origin
		// ----------
		USkySphere m_skySphere;

		EVisualizeOptions m_visualizeOption;
	};
}
