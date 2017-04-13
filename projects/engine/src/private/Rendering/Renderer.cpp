#include "Rendering/Renderer.h"
#include "Rendering/RenderPassProgram.h"
#include "Rendering/CameraInstance.h"

#include "Core/GameWindow.h"
#include "Misc/ProgramPermutor.h"
#include "Misc/Logging.h"
#include "Rendering/GraphicsDriver.h"
#include "Rendering/InputLayoutCache.h"
#include "Rendering/ParticleSystem/ParticleSystem.h"
#include "Rendering/RenderingConstants.h"

namespace MAD
{
	DECLARE_LOG_CATEGORY(LogRenderer);

	namespace
	{
		UGraphicsDriver g_graphicsDriver;
	}

	URenderer::URenderer(): m_frame(static_cast<decltype(m_frame)>(-1))
	                      , m_window(nullptr)
	                      , m_currentStateIndex(0)
	                      , m_visualizeOption(EVisualizeOptions::None)
						  , m_isDebugLayerEnabled(true) { }

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
		SetViewport(clientSize.x, clientSize.y);

		InitializeRenderPasses();
		InitializeDebugGrid(6);

		m_globalEnvironmentMap = UColorTextureCube(TexturePaths::EnvironmentMapTexture);
		m_skySphere = USkySphere(ShaderPaths::SkyboxPass, TexturePaths::EnvironmentMapTexture, Vector3(5000, 5000, 5000));

		m_textBatchRenderer.Init("engine\\fonts\\cambria_font.json", 1028);

		LOG(LogRenderer, Log, "Renderer initialization successful\n");
		return true;
	}

	void URenderer::OnScreenSizeChanged()
	{
		auto newSize = m_window->GetClientSize();
		LOG(LogRenderer, Log, "OnScreenSizeChanged: { %i, %i }\n", newSize.x, newSize.y);

		SetViewport(newSize.x, newSize.y);

		InitializeRenderPasses();

		m_textBatchRenderer.OnScreenSizeChanged();

		m_backBuffer.Reset(); // When we resize the window, we need to make sure we release all references to the back buffer (the graphics driver and renderer both have a reference)

		g_graphicsDriver.OnScreenSizeChanged();

		m_backBuffer = g_graphicsDriver.GetBackBufferRenderTarget();
	}

	void URenderer::InitializeRenderPasses()
	{
		InitializeGBufferPass(ShaderPaths::GBufferPass);
		InitializeDirectionalLightingPass(ShaderPaths::DeferredLightingPass);
		InitializePointLightingPass(ShaderPaths::DeferredLightingPass);
		InitializeDebugPass(ShaderPaths::DebugGeometryPass);
		InitializeTextRenderPass(ShaderPaths::DebugTextPass);

		InitializeDirectionalShadowMappingPass(ShaderPaths::DepthPass);
		InitializePointLightShadowMappingPass(ShaderPaths::DepthPass);
	}

	void URenderer::Shutdown()
	{
		g_graphicsDriver.Shutdown();
	}

	void URenderer::QueueDynamicDrawItem(const SDrawItem& inDrawItem)
	{
		// Queue up this draw item
		auto result = m_queuedDrawItems[m_currentStateIndex].insert({ inDrawItem.m_uniqueID, inDrawItem });
		MAD_ASSERT_DESC(result.second, "Duplicate draw item detected. Either it was submitted twice, or there was a collision in generating its unique ID");

		// Check if this draw item was submitted previously
		auto iter = m_queuedDrawItems[1 - m_currentStateIndex].find(inDrawItem.m_uniqueID);
		if (iter != m_queuedDrawItems[1 - m_currentStateIndex].end())
		{
			result.first->second.m_previousDrawTransform = &iter->second.m_transform;
		}
	}

	void URenderer::QueueStaticDrawItem(const SDrawItem& inDrawItem)
	{
		// Queue up this draw item
		auto result = m_staticQueuedItems.insert({ inDrawItem.m_uniqueID, inDrawItem });
		MAD_ASSERT_DESC(result.second, "Duplicate static draw item detected. Static draw items don't need to be re-queued every frame since their state should be the same across frames");
	}

	void URenderer::QueueDebugDrawItem(const SDrawItem& inDebugDrawItem, float inDuration /*= 0.0f*/)
	{
		SDebugHandle debugHandle;

		debugHandle.m_initialGameTime = gEngine->GetGameTimeDouble();
		debugHandle.m_duration = inDuration;
		debugHandle.m_debugDrawItem = inDebugDrawItem;

		m_debugDrawItems.emplace_back(debugHandle);
	}

	void URenderer::QueueDirectionalLight(size_t inID, const SGPUDirectionalLight& inDirectionalLight)
	{
		auto result = m_queuedDirLights[m_currentStateIndex].insert({ inID, inDirectionalLight });
		MAD_ASSERT_DESC(result.second, "Duplicate directional light detected. Either it was submitted twice, or there was a collision in generating its unique ID");

		SGPUDirectionalLight& newLight = result.first->second;

		const float sceneRadius = 2000.0f;
		const Matrix view = Matrix::CreateLookAt(-newLight.m_lightDirection, Vector3::Zero, Vector3::Up);
		const Matrix proj = Matrix::CreateOrthographic(2 * sceneRadius, 2 * sceneRadius, -1750.0f, 500.0f);

		newLight.m_viewProjectionMatrix = view * proj;
	}

	void URenderer::QueuePointLight(size_t inID, const SGPUPointLight& inPointLight)
	{
		auto result = m_queuedPointLights[m_currentStateIndex].insert({ inID, inPointLight });
		MAD_ASSERT_DESC(result.second, "Duplicate point light detected. Either it was submitted twice, or there was a collision in generating its unique ID");
		(void)result;
	}

	void URenderer::DrawDebugLine(const Vector3& inWSStart, const Vector3& inWSEnd, float inDuration, const Color& inLineColor /*= Color(1.0, 1.0, 1.0, 1.0)*/)
	{
		const Vector3 inMSStart(Vector3::Zero);
		const Vector3 inMSEnd(inWSEnd - inWSStart);

		// Construct draw item for line
		SDebugHandle lineDebugHandle;

		SDrawItem lineDrawItem;

		lineDrawItem.m_transform = ULinearTransform(); // Start with identity
		lineDrawItem.m_rasterizerState = m_debugPassDescriptor.m_rasterizerState;
		lineDrawItem.m_primitiveTopology = EPrimitiveTopology::LineList;
		lineDrawItem.m_vertexCount = 2;
		lineDrawItem.m_vertexBufferOffset = 0; // No sub-meshes obviously

		PopulateDebugLineVertices(inMSStart, inMSEnd, inLineColor, lineDrawItem.m_vertexBuffers);

		lineDrawItem.m_transform.SetTranslation(inWSStart);

		QueueDebugDrawItem(lineDrawItem, inDuration);
	}

	void URenderer::DrawOnScreenText(const eastl::string& inSourceString, float inScreenX, float inScreenY)
	{
		m_textBatchRenderer.BatchTextInstance(inSourceString, inScreenX, inScreenY);
	}

	UParticleSystem* URenderer::SpawnParticleSystem(const SParticleSystemSpawnParams& inSpawnParams, const eastl::vector<SParticleEmitterSpawnParams>& inEmitterParams)
	{
		return m_particleSystemManager.ActivateParticleSystem(inSpawnParams, inEmitterParams);
	}

	void URenderer::ClearRenderItems()
	{
		m_currentStateIndex = 1 - m_currentStateIndex;

		// Clear out the expired debug draw items
		ClearExpiredDebugDrawItems();

		// Keep static draw items around until requested to remove, always refresh dynamic draw items however
		m_queuedDrawItems[m_currentStateIndex].clear();
		m_queuedDirLights[m_currentStateIndex].clear();
		m_queuedPointLights[m_currentStateIndex].clear();
	}

	void URenderer::Frame(float inFramePercent, float inFrameTime)
	{
		m_frame++;
		MAD_ASSERT_DESC(m_frame != eastl::numeric_limits<decltype(m_frame)>::max(), "");
		
		BeginFrame();

		if (m_frame > 0)
		{
			// We need at least one frame's worth of buffer for state interpolation
			Draw(inFramePercent, inFrameTime);
		}
		
		EndFrame();
	}

	void URenderer::InitializeGBufferPass(const eastl::string& inGBufferProgramPath)
	{
		// G-Buffer textures will be the same size as the screen
		auto clientSize = m_window->GetClientSize();

		m_gBufferShaderResources.resize(AsIntegral(ETextureSlot::MAX) - AsIntegral(ETextureSlot::LightingBuffer));
		g_graphicsDriver.SetPixelShaderResource(nullptr, ETextureSlot::LightingBuffer);
		g_graphicsDriver.SetPixelShaderResource(nullptr, ETextureSlot::DiffuseBuffer);
		g_graphicsDriver.SetPixelShaderResource(nullptr, ETextureSlot::NormalBuffer);
		g_graphicsDriver.SetPixelShaderResource(nullptr, ETextureSlot::SpecularBuffer);
		g_graphicsDriver.SetPixelShaderResource(nullptr, ETextureSlot::DepthBuffer);
		g_graphicsDriver.SetPixelShaderResource(nullptr, ETextureSlot::ReflectionBuffer);

		for (unsigned i = 0; i < m_gBufferShaderResources.size(); ++i)
		{
			g_graphicsDriver.DestroyShaderResource(m_gBufferShaderResources[i]);
		}

		g_graphicsDriver.DestroyDepthStencil(m_gBufferPassDescriptor.m_depthStencilView);
		ShaderResourcePtr_t& depthBufferSRV = m_gBufferShaderResources[AsIntegral(ETextureSlot::DepthBuffer) - AsIntegral(ETextureSlot::LightingBuffer)];
		m_gBufferPassDescriptor.m_depthStencilView = g_graphicsDriver.CreateDepthStencil(clientSize.x, clientSize.y, &depthBufferSRV);
		m_gBufferPassDescriptor.m_depthStencilState = g_graphicsDriver.CreateDepthStencilState(true, EComparisonFunc::Less);

		for (unsigned i = 0; i < m_gBufferPassDescriptor.m_renderTargets.size(); ++i)
		{
			g_graphicsDriver.DestroyRenderTarget(m_gBufferPassDescriptor.m_renderTargets[i]);
		}
		
		ShaderResourcePtr_t& lightBufferSRV = m_gBufferShaderResources[AsIntegral(ETextureSlot::LightingBuffer) - AsIntegral(ETextureSlot::LightingBuffer)];
		ShaderResourcePtr_t& diffuseBufferSRV = m_gBufferShaderResources[AsIntegral(ETextureSlot::DiffuseBuffer) - AsIntegral(ETextureSlot::LightingBuffer)];
		ShaderResourcePtr_t& normalBufferSRV = m_gBufferShaderResources[AsIntegral(ETextureSlot::NormalBuffer) - AsIntegral(ETextureSlot::LightingBuffer)];
		ShaderResourcePtr_t& specularBufferSRV = m_gBufferShaderResources[AsIntegral(ETextureSlot::SpecularBuffer) - AsIntegral(ETextureSlot::LightingBuffer)];
		ShaderResourcePtr_t& reflectionBufferSRV = m_gBufferShaderResources[AsIntegral(ETextureSlot::ReflectionBuffer) - AsIntegral(ETextureSlot::LightingBuffer)];

		m_gBufferPassDescriptor.m_renderTargets.resize(AsIntegral(ERenderTargetSlot::MAX));
		m_gBufferPassDescriptor.m_renderTargets[AsIntegral(ERenderTargetSlot::LightingBuffer)] = g_graphicsDriver.CreateRenderTarget(clientSize.x, clientSize.y, DXGI_FORMAT_R16G16B16A16_FLOAT, &lightBufferSRV);
		m_gBufferPassDescriptor.m_renderTargets[AsIntegral(ERenderTargetSlot::DiffuseBuffer)] = g_graphicsDriver.CreateRenderTarget(clientSize.x, clientSize.y, DXGI_FORMAT_R8G8B8A8_UNORM, &diffuseBufferSRV);
		m_gBufferPassDescriptor.m_renderTargets[AsIntegral(ERenderTargetSlot::NormalBuffer)] = g_graphicsDriver.CreateRenderTarget(clientSize.x, clientSize.y, DXGI_FORMAT_R16G16_UNORM, &normalBufferSRV);
		m_gBufferPassDescriptor.m_renderTargets[AsIntegral(ERenderTargetSlot::SpecularBuffer)] = g_graphicsDriver.CreateRenderTarget(clientSize.x, clientSize.y, DXGI_FORMAT_R8G8B8A8_UNORM, &specularBufferSRV);
		m_gBufferPassDescriptor.m_renderTargets[AsIntegral(ERenderTargetSlot::ReflectionBuffer)] = g_graphicsDriver.CreateRenderTarget(clientSize.x, clientSize.y, DXGI_FORMAT_R16_FLOAT, &reflectionBufferSRV);

#ifdef _DEBUG
		g_graphicsDriver.SetDebugName_RenderTarget(g_graphicsDriver.GetBackBufferRenderTarget(), "Back Buffer");
		g_graphicsDriver.SetDebugName_RenderTarget(m_gBufferPassDescriptor.m_renderTargets[AsIntegral(ERenderTargetSlot::LightingBuffer)], "Light Accumulation Buffer");
		g_graphicsDriver.SetDebugName_RenderTarget(m_gBufferPassDescriptor.m_renderTargets[AsIntegral(ERenderTargetSlot::DiffuseBuffer)], "Diffuse Buffer");
		g_graphicsDriver.SetDebugName_RenderTarget(m_gBufferPassDescriptor.m_renderTargets[AsIntegral(ERenderTargetSlot::NormalBuffer)], "Normal Buffer");
		g_graphicsDriver.SetDebugName_RenderTarget(m_gBufferPassDescriptor.m_renderTargets[AsIntegral(ERenderTargetSlot::SpecularBuffer)], "Specular Buffer");
		g_graphicsDriver.SetDebugName_RenderTarget(m_gBufferPassDescriptor.m_renderTargets[AsIntegral(ERenderTargetSlot::ReflectionBuffer)], "Reflection Buffer");
#endif

		m_gBufferPassDescriptor.m_renderPassProgram = URenderPassProgram::Load(inGBufferProgramPath);

		m_gBufferPassDescriptor.m_blendState = g_graphicsDriver.CreateBlendState(false);
	}

	void URenderer::InitializeDirectionalLightingPass(const eastl::string& inDirLightingPassProgramPath)
	{
		m_dirLightingPassDescriptor.m_depthStencilView = nullptr;
		m_dirLightingPassDescriptor.m_depthStencilState = g_graphicsDriver.CreateDepthStencilState(false, EComparisonFunc::Always);

		m_dirLightingPassDescriptor.m_renderTargets.clear();
		m_dirLightingPassDescriptor.m_renderTargets.push_back(m_gBufferPassDescriptor.m_renderTargets[AsIntegral(ERenderTargetSlot::LightingBuffer)]);
		m_dirLightingPassDescriptor.m_renderTargets[AsIntegral(ERenderTargetSlot::LightingBuffer)] = m_gBufferPassDescriptor.m_renderTargets[AsIntegral(ERenderTargetSlot::LightingBuffer)];

		m_dirLightingPassDescriptor.m_rasterizerState = GetRasterizerState(EFillMode::Solid, ECullMode::Front);

		m_dirLightingPassDescriptor.m_renderPassProgram = URenderPassProgram::Load(inDirLightingPassProgramPath);

		m_dirLightingPassDescriptor.m_blendState = g_graphicsDriver.CreateBlendState(true);
	}

	void URenderer::InitializePointLightingPass(const eastl::string& inLightingPassProgramPath)
	{
		m_pointLightingPassDescriptor.m_depthStencilView = nullptr;
		m_pointLightingPassDescriptor.m_depthStencilState = g_graphicsDriver.CreateDepthStencilState(false, EComparisonFunc::Always);

		m_pointLightingPassDescriptor.m_renderTargets.clear();
		m_pointLightingPassDescriptor.m_renderTargets.push_back(m_gBufferPassDescriptor.m_renderTargets[AsIntegral(ERenderTargetSlot::LightingBuffer)]);
		m_pointLightingPassDescriptor.m_renderTargets[AsIntegral(ERenderTargetSlot::LightingBuffer)] = m_gBufferPassDescriptor.m_renderTargets[AsIntegral(ERenderTargetSlot::LightingBuffer)];

		m_pointLightingPassDescriptor.m_rasterizerState = GetRasterizerState(EFillMode::Solid, ECullMode::Front);

		m_pointLightingPassDescriptor.m_renderPassProgram = URenderPassProgram::Load(inLightingPassProgramPath);

		m_pointLightingPassDescriptor.m_blendState = g_graphicsDriver.CreateBlendState(true);
	}

	void URenderer::InitializeDebugPass(const eastl::string& inProgramPath)
	{
		// For the render target, we want to render directly into the light accumulation buffer
		m_debugPassDescriptor.m_renderTargets.clear();
		m_debugPassDescriptor.m_renderTargets.push_back(m_gBufferPassDescriptor.m_renderTargets[AsIntegral(ERenderTargetSlot::LightingBuffer)]);

		// The debug pass will use the same depth stencil view of the g-buffer pass since
		// we don't want debug lines to draw over pre-existing objects in the scene (unless they're non-occluded of course)
		m_debugPassDescriptor.m_depthStencilView = m_gBufferPassDescriptor.m_depthStencilView;
		m_debugPassDescriptor.m_depthStencilState = m_gBufferPassDescriptor.m_depthStencilState;

		if (!m_debugPassDescriptor.m_rasterizerState)
		{
			// Since we only want to draw lines/points, our rasterizer state needs to rasterizer using wireframe
			m_debugPassDescriptor.m_rasterizerState = GetRasterizerState(EFillMode::WireFrame, ECullMode::None);
		}

		m_debugPassDescriptor.m_renderPassProgram = URenderPassProgram::Load(inProgramPath);
		m_debugPassDescriptor.m_blendState = g_graphicsDriver.CreateBlendState(false);
	}

	void URenderer::InitializeTextRenderPass(const eastl::string& inProgramPath)
	{
		m_textRenderPassDescriptor.m_renderTargets.clear();
		m_textRenderPassDescriptor.m_renderTargets.push_back(m_gBufferPassDescriptor.m_renderTargets[AsIntegral(ERenderTargetSlot::LightingBuffer)]);

		// The text render pass will not use a depth stencil view because there is no concept of depth with text (we always want text to be on top of everything (potentially not in the future?))
		m_textRenderPassDescriptor.m_depthStencilView = nullptr;
		m_textRenderPassDescriptor.m_depthStencilState = nullptr;

		if (!m_textRenderPassDescriptor.m_rasterizerState)
		{
			m_textRenderPassDescriptor.m_rasterizerState = GetRasterizerState(EFillMode::Solid, ECullMode::None);
		}

		m_textRenderPassDescriptor.m_renderPassProgram = URenderPassProgram::Load(inProgramPath);
		m_textRenderPassDescriptor.m_blendState = g_graphicsDriver.CreateBlendState(true);
	}

	void URenderer::InitializeDirectionalShadowMappingPass(const eastl::string& inProgramPath)
	{
		g_graphicsDriver.DestroyDepthStencil(m_dirShadowMappingPassDescriptor.m_depthStencilView);
		m_dirShadowMappingPassDescriptor.m_depthStencilView = g_graphicsDriver.CreateDepthStencil(4096, 4096, &m_shadowMapSRV);
		m_dirShadowMappingPassDescriptor.m_depthStencilState = g_graphicsDriver.CreateDepthStencilState(true, EComparisonFunc::Less);

		m_dirShadowMappingPassDescriptor.m_blendState = g_graphicsDriver.CreateBlendState(false);

		if (!m_dirShadowMappingPassDescriptor.m_rasterizerState)
		{
			m_dirShadowMappingPassDescriptor.m_rasterizerState = g_graphicsDriver.CreateDepthRasterizerState();
		}

		m_dirShadowMappingPassDescriptor.m_renderPassProgram = URenderPassProgram::Load(inProgramPath);
	}

	void URenderer::InitializePointLightShadowMappingPass(const eastl::string& inProgramPath)
	{
		m_depthTextureCube = eastl::make_unique<UDepthTextureCube>(static_cast<uint16_t>(4096));

		g_graphicsDriver.DestroyDepthStencil(m_pointShadowMappingPassDescriptor.m_depthStencilView);
		m_pointShadowMappingPassDescriptor.m_depthStencilState = g_graphicsDriver.CreateDepthStencilState(true, EComparisonFunc::Less);
		m_pointShadowMappingPassDescriptor.m_blendState = g_graphicsDriver.CreateBlendState(false);

		if (!m_pointShadowMappingPassDescriptor.m_rasterizerState)
		{
			m_pointShadowMappingPassDescriptor.m_rasterizerState = g_graphicsDriver.CreateDepthRasterizerState();
		}

		m_pointShadowMappingPassDescriptor.m_renderPassProgram = URenderPassProgram::Load(inProgramPath);
	}

	void URenderer::InitializeDebugGrid(uint8_t inGridDimension)
	{
		// Draw all of the debug lines required for the grid
		const float coordinateScale = 100.0f;
		const float gridYLiftThreshold = 0.25f;
		Vector3 currentLineBegin = { -coordinateScale * ceilf(inGridDimension / 2.0f), gridYLiftThreshold, -coordinateScale * ceilf(inGridDimension / 2.0f) };
		Vector3 currentLineEnd = { -coordinateScale * ceilf(inGridDimension / 2.0f), gridYLiftThreshold, coordinateScale * ceilf(inGridDimension / 2.0f) };

		// Vertical lines
		for (uint8_t i = 0; i <= inGridDimension; ++i)
		{
			DrawDebugLine(currentLineBegin, currentLineEnd, -1.0f);

			currentLineBegin.x += coordinateScale;
			currentLineEnd.x += coordinateScale;
		}

		// Horizontal lines
		currentLineBegin = Vector3({ -coordinateScale * ceilf(inGridDimension / 2.0f), gridYLiftThreshold, -coordinateScale * ceilf(inGridDimension / 2.0f) });
		currentLineEnd = Vector3({ coordinateScale * ceilf(inGridDimension / 2.0f), gridYLiftThreshold, -coordinateScale * ceilf(inGridDimension / 2.0f) });

		for (uint8_t i = 0; i <= inGridDimension; ++i)
		{
			DrawDebugLine(currentLineBegin, currentLineEnd, -1.0f);

			currentLineBegin.z += coordinateScale;
			currentLineEnd.z += coordinateScale;
		}
	}

	void URenderer::PopulatePointShadowVPMatrices(const Vector3& inWSLightPos, TextureCubeVPArray_t& inOutVPArray)
	{
		// Use the world space basis axis
		const Matrix  perspectiveProjMatrix = Matrix::CreatePerspectiveFieldOfView(DirectX::XM_PIDIV2, 1.0f, 50.0f, 100000.0f); // We have to make sure that the near and far planes are proportional to the world units

		// Calculate the points to look at for each direction
		const Vector3 wsDirectionTargets[UDepthTextureCube::s_numTextureCubeSides] =
		{
			{ inWSLightPos.x + 1.0f, inWSLightPos.y       , inWSLightPos.z        }, // +X
			{ inWSLightPos.x - 1.0f, inWSLightPos.y       , inWSLightPos.z        }, // -X
			{ inWSLightPos.x       , inWSLightPos.y + 1.0f, inWSLightPos.z        }, // +Y
			{ inWSLightPos.x       , inWSLightPos.y - 1.0f, inWSLightPos.z        }, // -Y
			{ inWSLightPos.x       , inWSLightPos.y       , inWSLightPos.z - 1.0f }, // +Z
			{ inWSLightPos.x       , inWSLightPos.y       , inWSLightPos.z + 1.0f }, // -Z
		};

		const Vector3 wsUpVectors[UDepthTextureCube::s_numTextureCubeSides] =
		{
			{ 0.0f, 1.0f, 0.0f }, // +X
			{ 0.0f, 1.0f, 0.0f },  // -X
			{ 0.0f, 0.0f, 1.0f },  // +Y (use a different up)
			{ 0.0f, 0.0f, -1.0f },  // -Y (use a different up)
			{ 0.0f, 1.0f, 0.0f },  // +Z
			{ 0.0f, 1.0f, 0.0f }   // -Z
		};

		for (uint8_t i = 0; i < UDepthTextureCube::s_numTextureCubeSides; ++i)
		{
			inOutVPArray[i] = Matrix::CreateLookAt(inWSLightPos, wsDirectionTargets[i], wsUpVectors[i]) * perspectiveProjMatrix;
		}
	}

	void URenderer::PopulateDebugLineVertices(const Vector3& inMSStart, const Vector3& inMSEnd, const Color& inLineColor, eastl::vector<UVertexArray>& inOutLineVertexData)
	{
		inOutLineVertexData.clear();

		// Create two vertex arrays (one for position, one for color)
		Vector3 vertexPositionData[2] = { inMSStart, inMSEnd };
		Vector3 vertexColorData[2] = { Vector3(inLineColor.R(), inLineColor.G(), inLineColor.B()), Vector3(inLineColor.R(), inLineColor.G(), inLineColor.B()) };

		inOutLineVertexData.emplace_back(g_graphicsDriver, EVertexBufferSlot::Position, EInputLayoutSemantic::Position, vertexPositionData, static_cast<uint32_t>(sizeof(Vector3)), 2);
		inOutLineVertexData.emplace_back(g_graphicsDriver, EVertexBufferSlot::Normal, EInputLayoutSemantic::Normal, vertexColorData, static_cast<uint32_t>(sizeof(Vector3)), 2);
	}

	void URenderer::BindPerFrameConstants()
	{
		// Update the per frame constant buffer
		g_graphicsDriver.UpdateBuffer(EConstantBufferSlot::PerFrame, &m_perFrameConstants, sizeof(m_perFrameConstants));
	}

	void URenderer::SetViewport(LONG inWidth, LONG inHeight)
	{
		float w = static_cast<float>(inWidth);
		float h = static_cast<float>(inHeight);

		m_perSceneConstants.m_screenDimensions.x = w;
		m_perSceneConstants.m_screenDimensions.y = h;
		g_graphicsDriver.UpdateBuffer(EConstantBufferSlot::PerScene, &m_perSceneConstants, sizeof(m_perSceneConstants));

		g_graphicsDriver.SetViewport(0, 0, w, h);
	}

	void URenderer::BeginFrame()
	{
		static const float zero[] = { 0.0f, 0.0f, 0.0f, 0.0f };

		g_graphicsDriver.StartEventGroup(L"Clear render targets and depth");

		g_graphicsDriver.ClearDepthStencil(m_gBufferPassDescriptor.m_depthStencilView, true, 1.0f);

		// Clear the light accumulation buffer to the screen clear color
		g_graphicsDriver.ClearRenderTarget(m_gBufferPassDescriptor.m_renderTargets[0], m_clearColor);

		// Clear other G-buffers to 0
		for (unsigned i = 1; i < m_gBufferPassDescriptor.m_renderTargets.size(); ++i)
		{
			auto renderTarget = m_gBufferPassDescriptor.m_renderTargets[i];
			g_graphicsDriver.ClearRenderTarget(renderTarget, zero);
		}

		g_graphicsDriver.EndEventGroup();
	}

	void URenderer::Draw(float inFramePercent, float inFrameTime)
	{
		// Bind per-frame constants
		CalculateCameraConstants(inFramePercent);
		m_perFrameConstants.m_gameTime = gEngine->GetGameTime();
		m_perFrameConstants.m_frameTime = inFrameTime;
		BindPerFrameConstants();

		m_globalEnvironmentMap.BindAsResource(ETextureSlot::CubeMap);

		DrawGBuffer(inFramePercent);
		DrawDirectionalLighting(inFramePercent);
		DrawPointLighting(inFramePercent);

		m_skySphere.DrawSkySphere();

		// Always perform the forward debug pass after the main deferred pass
		DrawDebugPrimitives(inFramePercent);

		// Always draw text as the last pass (WARNING, if we ever get to post processing effects, need to move this after the post processing)
		m_textRenderPassDescriptor.ApplyPassState(g_graphicsDriver);
		m_textRenderPassDescriptor.m_renderPassProgram->SetProgramActive(g_graphicsDriver, 0);
		m_textBatchRenderer.FlushBatch();

		g_graphicsDriver.StartEventGroup(L"Particle Systems");

		m_particleSystemManager.UpdateParticleSystems(inFrameTime);

		g_graphicsDriver.EndEventGroup();
	}

	void URenderer::EndFrame()
	{
		static bool loadedBackBufferProgram = false;
		static eastl::shared_ptr<URenderPassProgram> backBufferProgram;
		if (!loadedBackBufferProgram)
		{
			backBufferProgram = URenderPassProgram::Load(ShaderPaths::BackBufferFinalizePass);
			loadedBackBufferProgram = true;
		}

		g_graphicsDriver.StartEventGroup(L"Copy lighting accumulation to back buffer");

		// Copy the finalized linear lighting buffer to the back buffer
		// This (will) perform HDR lighting corrections and already performs gamma correction
		g_graphicsDriver.SetRenderTargets(&m_backBuffer, 1, nullptr);
		g_graphicsDriver.SetPixelShaderResource(m_gBufferShaderResources[AsIntegral(ETextureSlot::LightingBuffer) - AsIntegral(ETextureSlot::LightingBuffer)], ETextureSlot::LightingBuffer);
		g_graphicsDriver.SetBlendState(nullptr);
		backBufferProgram->SetProgramActive(g_graphicsDriver, 0);
		g_graphicsDriver.DrawFullscreenQuad();

		g_graphicsDriver.EndEventGroup();

		g_graphicsDriver.Present();
	}

	void URenderer::ClearExpiredDebugDrawItems()
	{
		const float currentGameTime = gEngine->GetGameTimeDouble();

		m_debugDrawItems.erase(eastl::remove_if(m_debugDrawItems.begin(), m_debugDrawItems.end(), [currentGameTime](const SDebugHandle& inDebugHandle)
		{
			if (inDebugHandle.m_duration < 0.0f)
			{
				return false;
			}

			return (currentGameTime - inDebugHandle.m_initialGameTime) > inDebugHandle.m_duration;
		}), m_debugDrawItems.end());
	}

	void URenderer::DrawGBuffer(float inFramePercent)
	{
		g_graphicsDriver.SetPixelShaderResource(nullptr, ETextureSlot::LightingBuffer);
		g_graphicsDriver.SetPixelShaderResource(nullptr, ETextureSlot::DiffuseBuffer);
		g_graphicsDriver.SetPixelShaderResource(nullptr, ETextureSlot::NormalBuffer);
		g_graphicsDriver.SetPixelShaderResource(nullptr, ETextureSlot::SpecularBuffer);
		g_graphicsDriver.SetPixelShaderResource(nullptr, ETextureSlot::DepthBuffer);
		g_graphicsDriver.SetPixelShaderResource(nullptr, ETextureSlot::ReflectionBuffer);

		m_gBufferPassDescriptor.ApplyPassState(g_graphicsDriver);

		// Go through each draw item and bind input assembly data
		g_graphicsDriver.StartEventGroup(L"Draw scene to GBuffer");
		for (auto& currentDrawItem : m_queuedDrawItems[m_currentStateIndex])
		{
			// Before processing the draw item, we need to determine which program it should use and bind that
			m_gBufferPassDescriptor.m_renderPassProgram->SetProgramActive(g_graphicsDriver, DetermineProgramId(currentDrawItem.second));

			// Each individual DrawItem should issue its own draw call
			currentDrawItem.second.Draw(g_graphicsDriver, inFramePercent, m_perFrameConstants, true);
		}
		g_graphicsDriver.EndEventGroup();

		if (m_visualizeOption != EVisualizeOptions::None)
		{
			g_graphicsDriver.StartEventGroup(L"Visualize GBuffer");
			DoVisualizeGBuffer();
			g_graphicsDriver.EndEventGroup();
			return;
		}

	}

	void URenderer::DrawDirectionalLighting(float inFramePercent)
	{
		g_graphicsDriver.StartEventGroup(L"Accumulate deferred directional lighting");

		g_graphicsDriver.SetPixelShaderResource(nullptr, ETextureSlot::DiffuseMap);

		m_dirShadowMappingPassDescriptor.ApplyPassState(g_graphicsDriver);
		g_graphicsDriver.SetPixelShaderResource(m_gBufferShaderResources[AsIntegral(ETextureSlot::DiffuseBuffer) - AsIntegral(ETextureSlot::LightingBuffer)], ETextureSlot::DiffuseBuffer);
		g_graphicsDriver.SetPixelShaderResource(m_gBufferShaderResources[AsIntegral(ETextureSlot::NormalBuffer) - AsIntegral(ETextureSlot::LightingBuffer)], ETextureSlot::NormalBuffer);
		g_graphicsDriver.SetPixelShaderResource(m_gBufferShaderResources[AsIntegral(ETextureSlot::SpecularBuffer) - AsIntegral(ETextureSlot::LightingBuffer)], ETextureSlot::SpecularBuffer);
		g_graphicsDriver.SetPixelShaderResource(m_gBufferShaderResources[AsIntegral(ETextureSlot::DepthBuffer) - AsIntegral(ETextureSlot::LightingBuffer)], ETextureSlot::DepthBuffer);
		g_graphicsDriver.SetPixelShaderResource(m_gBufferShaderResources[AsIntegral(ETextureSlot::ReflectionBuffer) - AsIntegral(ETextureSlot::LightingBuffer)], ETextureSlot::ReflectionBuffer);

		uint32_t currentDirectionalLightIndex = 0;

		SGPUDirectionalLight directionalLightConstants;
		for (const auto& currentDirLight : m_queuedDirLights[m_currentStateIndex])
		{
			g_graphicsDriver.StartEventGroup(eastl::wstring(eastl::wstring::CtorSprintf(), L"Directional light #%d", ++currentDirectionalLightIndex));

			// Interpolate the light's properties
			const auto previousDirLight = m_queuedDirLights[1 - m_currentStateIndex].find(currentDirLight.first);
			if (previousDirLight != m_queuedDirLights[1 - m_currentStateIndex].end())
			{
				directionalLightConstants = SGPUDirectionalLight::Lerp(previousDirLight->second, currentDirLight.second, inFramePercent);
			}
			else
			{
				directionalLightConstants = currentDirLight.second;
			}

			// Transform the light's direction into view space
			directionalLightConstants.m_lightDirection = Vector3::TransformNormal(directionalLightConstants.m_lightDirection, m_perFrameConstants.m_cameraViewMatrix);
			g_graphicsDriver.UpdateBuffer(EConstantBufferSlot::PerDirectionalLight, &directionalLightConstants, sizeof(SGPUDirectionalLight));

			// Render shadow map
			g_graphicsDriver.StartEventGroup(L"Draw scene to shadow map");
			g_graphicsDriver.SetPixelShaderResource(nullptr, ETextureSlot::DiffuseMap);
			m_dirShadowMappingPassDescriptor.ApplyPassState(g_graphicsDriver);
			m_dirShadowMappingPassDescriptor.m_renderPassProgram->SetProgramActive(g_graphicsDriver, static_cast<ProgramId_t>(EProgramIdMask::Lighting_DirectionalLight));

			g_graphicsDriver.ClearDepthStencil(m_dirShadowMappingPassDescriptor.m_depthStencilView, true, 1.0);
			g_graphicsDriver.SetViewport(0, 0, 4096, 4096);
			for (auto& currentDrawItem : m_queuedDrawItems[m_currentStateIndex])
			{
				currentDrawItem.second.Draw(g_graphicsDriver, inFramePercent, m_perFrameConstants, false, EInputLayoutSemantic::Position, m_dirShadowMappingPassDescriptor.m_rasterizerState);
			}

			g_graphicsDriver.EndEventGroup();

			// Shading + lighting
			m_dirLightingPassDescriptor.ApplyPassState(g_graphicsDriver);
			g_graphicsDriver.SetPixelShaderResource(m_shadowMapSRV, ETextureSlot::DiffuseMap);
			m_dirLightingPassDescriptor.m_renderPassProgram->SetProgramActive(g_graphicsDriver, static_cast<ProgramId_t>(EProgramIdMask::Lighting_DirectionalLight));
			g_graphicsDriver.SetViewport(0, 0, m_perSceneConstants.m_screenDimensions.x, m_perSceneConstants.m_screenDimensions.y);
			g_graphicsDriver.DrawFullscreenQuad();

			g_graphicsDriver.EndEventGroup();
		}
		g_graphicsDriver.EndEventGroup();
	}

	void URenderer::DrawPointLighting(float inFramePercent)
	{
		uint32_t currentPointLightIndex = 0;
		SPerPointLightConstants pointLightConstants;
		TextureCubeVPArray_t shadowMapVPMatrices;

		g_graphicsDriver.StartEventGroup(L"Accumulate deferred point lighting");

		m_pointLightingPassDescriptor.ApplyPassState(g_graphicsDriver);
		m_pointLightingPassDescriptor.m_renderPassProgram->SetProgramActive(g_graphicsDriver, static_cast<ProgramId_t>(EProgramIdMask::Lighting_PointLight));

		for (const auto& currentPointLight : m_queuedPointLights[m_currentStateIndex])
		{
			g_graphicsDriver.StartEventGroup(eastl::wstring(eastl::wstring::CtorSprintf(), L"Point light #%d", ++currentPointLightIndex));

			// Interpolate the light's properties
			const auto previousPointLight = m_queuedPointLights[1 - m_currentStateIndex].find(currentPointLight.first);
			if (previousPointLight != m_queuedPointLights[1 - m_currentStateIndex].end())
			{
				pointLightConstants.m_pointLight = SGPUPointLight::Lerp(previousPointLight->second, currentPointLight.second, inFramePercent);
			}
			else
			{
				pointLightConstants.m_pointLight = currentPointLight.second;
			}

			// TODO Inefficient, we should just calculate once for each point light once as long as it doesn't change position
			PopulatePointShadowVPMatrices(pointLightConstants.m_pointLight.m_lightPosition, shadowMapVPMatrices);

			memcpy(pointLightConstants.m_pointLightVPMatrices, shadowMapVPMatrices.data(), shadowMapVPMatrices.size() * sizeof(Matrix));

			// Clear the resource slot for the texture cube
			g_graphicsDriver.SetPixelShaderResource(nullptr, ETextureSlot::CubeMap);

			m_pointShadowMappingPassDescriptor.ApplyPassState(g_graphicsDriver);
			m_pointShadowMappingPassDescriptor.m_renderPassProgram->SetProgramActive(g_graphicsDriver, static_cast<ProgramId_t>(EProgramIdMask::Lighting_PointLight));

			for (int i = 0; i < 6; ++i)
			{
				g_graphicsDriver.StartEventGroup(eastl::wstring(eastl::wstring::CtorSprintf(), L"Shadow Cube Side #%d", i));

				// Bind the current side of the shadow texture cube
				m_depthTextureCube->BindCubeSideAsTarget(i);

				// Update the view-projection matrix of the current cube side in the per point light constant buffer
				pointLightConstants.m_pointLight.m_viewProjectionMatrix = shadowMapVPMatrices[i];
				g_graphicsDriver.UpdateBuffer(EConstantBufferSlot::PerPointLight, &pointLightConstants, sizeof(pointLightConstants));

				// Process all of the draw items again
				for (auto& currentDrawItem : m_queuedDrawItems[m_currentStateIndex])
				{
					currentDrawItem.second.Draw(g_graphicsDriver, inFramePercent, m_perFrameConstants, false, EInputLayoutSemantic::Position, m_pointShadowMappingPassDescriptor.m_rasterizerState);
				}

				g_graphicsDriver.EndEventGroup();
			}

			// Reset the viewport back to normal
			g_graphicsDriver.SetViewport(0, 0, m_perSceneConstants.m_screenDimensions.x, m_perSceneConstants.m_screenDimensions.y);

			// Bind the texture cube as shader resource
			m_depthTextureCube->BindAsResource(ETextureSlot::CubeMap);

			// Transform the light's position into view space
			pointLightConstants.m_pointLight.m_lightPosition = Vector3::Transform(pointLightConstants.m_pointLight.m_lightPosition, m_perFrameConstants.m_cameraViewMatrix);
			g_graphicsDriver.UpdateBuffer(EConstantBufferSlot::PerPointLight, &pointLightConstants, sizeof(pointLightConstants));

			m_pointLightingPassDescriptor.ApplyPassState(g_graphicsDriver);
			m_pointLightingPassDescriptor.m_renderPassProgram->SetProgramActive(g_graphicsDriver, static_cast<ProgramId_t>(EProgramIdMask::Lighting_PointLight));

			// Instead of just drawing a full screen quad, calculate the rectangle bounds (in NDC) for the current point light
			const float lightHalfWidth = pointLightConstants.m_pointLight.m_lightOuterRadius;
			const float lightHalfHeight = lightHalfWidth;

			Vector4 lightMinPosHS = { pointLightConstants.m_pointLight.m_lightPosition.x,
									  pointLightConstants.m_pointLight.m_lightPosition.y,
									  pointLightConstants.m_pointLight.m_lightPosition.z,
									  1.0f };

			Vector4 lightMaxPosHS = lightMinPosHS;

			// Calculate the min and the max extents in the x and y axis for the light in view space (so we can transform them into NDC space)
			lightMinPosHS.x -= lightHalfWidth;
			lightMinPosHS.y -= lightHalfHeight;

			lightMaxPosHS.x += lightHalfWidth;
			lightMaxPosHS.y += lightHalfHeight;

			lightMinPosHS = Vector4::Transform(lightMinPosHS, m_perFrameConstants.m_cameraProjectionMatrix);
			lightMaxPosHS = Vector4::Transform(lightMaxPosHS, m_perFrameConstants.m_cameraProjectionMatrix);

			// Check for zero w value because for points that at the same z-position as the camera, they will
			// lead to a 0 w-value (in viewspace) since the projection value produces a w value equal to -z
			if (lightMinPosHS.w != 0.0f && lightMaxPosHS.w != 0.0f)
			{
				// Perspective divide from clip into NDC space
				lightMinPosHS /= lightMinPosHS.w;
				lightMaxPosHS /= lightMaxPosHS.w;

				MAD_ASSERT_DESC(FloatEqual(lightMinPosHS.w, 1.0f), "Light min extent NDC position doesn't have a 1.0f w component, matrix transformation is incorrect!\n");
				MAD_ASSERT_DESC(FloatEqual(lightMaxPosHS.w, 1.0f), "Light max extent NDC position doesn't have a 1.0f w component, matrix transformation is incorrect!\n");

				// Light quad extents are in NDC
				g_graphicsDriver.DrawSubscreenQuad(lightMinPosHS, lightMaxPosHS);
			}

			g_graphicsDriver.EndEventGroup();
		}

		g_graphicsDriver.EndEventGroup();
	}

	void URenderer::DrawDebugPrimitives(float inFramePerecent)
	{
		if (!m_isDebugLayerEnabled)
		{
			return;
		}

		g_graphicsDriver.StartEventGroup(L"Debug Pass Layer");

		// Make sure the g buffer depth stencil shader resource view is not bound because we're using it as our depth stencil view here
		g_graphicsDriver.SetPixelShaderResource(nullptr, ETextureSlot::DepthBuffer);

		m_debugPassDescriptor.ApplyPassState(g_graphicsDriver);
		m_debugPassDescriptor.m_renderPassProgram->SetProgramActive(g_graphicsDriver, 0);

		// Process the debug draw items
		for (auto& currentDebugDrawItem : m_debugDrawItems)
		{
			currentDebugDrawItem.m_debugDrawItem.Draw(g_graphicsDriver, inFramePerecent, m_perFrameConstants, false);
		}

		g_graphicsDriver.EndEventGroup();
	}

	void URenderer::DoVisualizeGBuffer()
	{
		static bool loadedCopyTextureProgram = false;
		static eastl::shared_ptr<URenderPassProgram> copyTextureProgram;
		if (!loadedCopyTextureProgram)
		{
			copyTextureProgram = URenderPassProgram::Load(ShaderPaths::TextureBlitPass);
			loadedCopyTextureProgram = true;
		}

		static bool loadedDepthProgram = false;
		static eastl::shared_ptr<URenderPassProgram> depthProgram;
		if (!loadedDepthProgram)
		{
			depthProgram = URenderPassProgram::Load(ShaderPaths::DepthPass);
			loadedDepthProgram = true;
		}

		ShaderResourcePtr_t target;
		switch(m_visualizeOption)
		{
		case EVisualizeOptions::LightAccumulation:
			// Already in the back buffer, so just return
			return;
		case EVisualizeOptions::Diffuse:
			target = m_gBufferShaderResources[1];
			break;
		case EVisualizeOptions::Specular:
			target = m_gBufferShaderResources[3];
			break;
		case EVisualizeOptions::Normals:
			target = m_gBufferShaderResources[2];
			break;
		case EVisualizeOptions::Depth:
			target = m_gBufferShaderResources[4];
			break;
		default: return;
		}

		if (m_visualizeOption == EVisualizeOptions::Depth)
		{
			depthProgram->SetProgramActive(g_graphicsDriver, 0);
		}
		else
		{
			copyTextureProgram->SetProgramActive(g_graphicsDriver, 0);
		}

		auto& renderTarget = m_gBufferPassDescriptor.m_renderTargets[AsIntegral(ERenderTargetSlot::LightingBuffer)];
		g_graphicsDriver.SetRenderTargets(&renderTarget, 1, nullptr);
		g_graphicsDriver.SetPixelShaderResource(target, ETextureSlot::DiffuseBuffer);

		g_graphicsDriver.DrawFullscreenQuad();
	}

	ProgramId_t URenderer::DetermineProgramId(const SDrawItem& inTargetDrawItem) const
	{
		ProgramId_t outputProgramId = 0;

		for (const auto& currentTextureSlot : inTargetDrawItem.m_shaderResources)
		{
			switch (currentTextureSlot.first)
			{
			case ETextureSlot::DiffuseMap: // Do you have a diffuse map?
				outputProgramId |= static_cast<ProgramId_t>(EProgramIdMask::GBuffer_Diffuse);
				break;
			case ETextureSlot::SpecularMap: // Do you have a specular map?
				outputProgramId |= static_cast<ProgramId_t>(EProgramIdMask::GBuffer_Specular);
				break;
			case ETextureSlot::EmissiveMap: // Do you have a emissive map?
				outputProgramId |= static_cast<ProgramId_t>(EProgramIdMask::GBuffer_Emissive);
				break;
			case ETextureSlot::OpacityMask: // Do you have an opacity mask?
				outputProgramId |= static_cast<ProgramId_t>(EProgramIdMask::GBuffer_OpacityMask);
				break;
			case ETextureSlot::NormalMap: // Do you have a normal map?
				outputProgramId |= static_cast<ProgramId_t>(EProgramIdMask::GBuffer_NormalMap);
				break;
			}
		}

		return outputProgramId;
	}

	void URenderer::SetFullScreen(bool inIsFullscreen) const
	{
		g_graphicsDriver.SetFullScreen(inIsFullscreen);
	}

	void URenderer::UpdateCameraConstants(const SCameraInstance& inCameraInstance)
	{
		m_camera[m_currentStateIndex] = inCameraInstance;
	}

	void URenderer::CalculateCameraConstants(float inFramePercent)
	{
		SCameraInstance& currentCamera = m_camera[m_currentStateIndex];
		ULinearTransform transform = ULinearTransform::Lerp(m_camera[1 - m_currentStateIndex].m_transform, currentCamera.m_transform, inFramePercent);

		auto windowSize = gEngine->GetWindow().GetClientSize();
		float aspectRatio = static_cast<float>(windowSize.x) / windowSize.y;

		Matrix projection = Matrix::CreatePerspectiveFieldOfView(currentCamera.m_verticalFOV, aspectRatio, currentCamera.m_nearPlaneDistance, currentCamera.m_farPlaneDistance);

		m_perFrameConstants.m_cameraViewMatrix = transform.GetMatrix().Invert();
		m_perFrameConstants.m_cameraProjectionMatrix = projection;
		m_perFrameConstants.m_cameraViewProjectionMatrix = m_perFrameConstants.m_cameraViewMatrix * m_perFrameConstants.m_cameraProjectionMatrix;
		m_perFrameConstants.m_cameraInverseViewMatrix = transform.GetMatrix();
		m_perFrameConstants.m_cameraInverseProjectionMatrix = projection.Invert();
		m_perFrameConstants.m_cameraNearPlane = currentCamera.m_nearPlaneDistance;
		m_perFrameConstants.m_cameraFarPlane = currentCamera.m_farPlaneDistance;
		m_perFrameConstants.m_cameraExposure = currentCamera.m_exposure;
	}

	void URenderer::SetWorldAmbientColor(const Color& inColor)
	{
		// Convert from sRGB space to linear color space.
		// This is then converted back to sRGB space when rendering to the back buffer
		// and then back to linear by the monitor...
		m_perSceneConstants.m_ambientColor.x = powf(inColor.x, 2.2f);
		m_perSceneConstants.m_ambientColor.y = powf(inColor.y, 2.2f);
		m_perSceneConstants.m_ambientColor.z = powf(inColor.z, 2.2f);
		m_perSceneConstants.m_ambientColor.w = 1.0f;

		g_graphicsDriver.UpdateBuffer(EConstantBufferSlot::PerScene, &m_perSceneConstants, sizeof(m_perSceneConstants));
	}

	void URenderer::SetBackBufferClearColor(const Color& inColor)
	{
		m_clearColor.x = powf(inColor.x, 2.2f);
		m_clearColor.y = powf(inColor.y, 2.2f);
		m_clearColor.z = powf(inColor.z, 2.2f);
		m_clearColor.w = 1.0f;
	}

	class UGraphicsDriver& URenderer::GetGraphicsDriver()
	{
		return g_graphicsDriver;
	}

	RasterizerStatePtr_t URenderer::GetRasterizerState(EFillMode inFillMode, ECullMode inCullMode) const
	{
		static eastl::hash_map<uint32_t, RasterizerStatePtr_t> s_stateCache;

		uint32_t hash = static_cast<uint32_t>(inFillMode) + static_cast<uint32_t>(inCullMode) * 17;
		auto iter = s_stateCache.find(hash);
		if (iter != s_stateCache.end())
		{
			return iter->second;
		}

		auto state = g_graphicsDriver.CreateRasterizerState(inFillMode, inCullMode);
		s_stateCache.insert({ hash, state });
		return state;
	}

	void URenderer::ToggleTextBatching()
	{
		m_textBatchRenderer.SetTextBatchingEnabled(!m_textBatchRenderer.GetTextBatchingEnabled());
	}

	POINT URenderer::GetScreenSize() const
	{
		auto screenSize = m_window->GetClientSize();

		return { screenSize.x, screenSize.y };
	}

}
