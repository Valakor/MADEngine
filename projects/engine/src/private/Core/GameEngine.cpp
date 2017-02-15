#include "Core/GameEngine.h"

#include <EASTL/algorithm.h>

#include "Core/FrameTimer.h"
#include "Core/GameInput.h"
#include "Core/GameInstance.h"
#include "Core/GameWindow.h"
#include "Core/GameWorld.h"
#include "Core/GameWorldLoader.h"
#include "Core/PhysicsWorld.h"
#include "Misc/AssetCache.h"
#include "Rendering/Renderer.h"

#include "Core/Character.h"
#include "Core/CameraComponent.h"
#include "Core/MeshComponent.h"
#include "Core/LightComponent.h"
#include "Core/DirectionalLightComponent.h"
#include "Core/PointLightComponent.h"
#include "Core/DebugTransformComponent.h"

#include "Networking/NetworkState.h"

// TESTING
#include "Testing/TestCharacters.h"
#include "Testing/TestComponents.h"
#include "Testing/EntityTestingModule.h"

#include "Rendering/FontFamily.h"

using eastl::string;

namespace MAD
{
	DECLARE_LOG_CATEGORY(LogGameEngine);

	namespace
	{
		// Temp testing to change render target output for GBuffer
		void OnDisableGBufferVisualization()
		{
			gEngine->GetRenderer().SetGBufferVisualizeOption(EVisualizeOptions::None);
		}

		void OnEnableLightAccumulation()
		{
			gEngine->GetRenderer().SetGBufferVisualizeOption(EVisualizeOptions::LightAccumulation);
		}

		void OnEnableDiffuse()
		{
			gEngine->GetRenderer().SetGBufferVisualizeOption(EVisualizeOptions::Diffuse);
		}

		void OnEnableSpecular()
		{
			gEngine->GetRenderer().SetGBufferVisualizeOption(EVisualizeOptions::Specular);
		}

		void OnEnableNormals()
		{
			gEngine->GetRenderer().SetGBufferVisualizeOption(EVisualizeOptions::Normals);
		}

		void OnEnableDepth()
		{
			gEngine->GetRenderer().SetGBufferVisualizeOption(EVisualizeOptions::Depth);
		}

		void OnEnableDebugPrimitives()
		{
			gEngine->GetRenderer().ToggleDebugLayerEnabled();
		}

		void OnToggleHUD()
		{
			gEngine->GetRenderer().ToggleTextBatching();
		}
	}

	UGameEngine::~UGameEngine()
	{
		m_networkManager.Shutdown();

		m_worlds.clear();

		m_gameInstance->OnShutdown();
		m_gameInstance = nullptr;

		m_renderer->Shutdown();
		m_renderer = nullptr;

		m_gameWindow = nullptr;

		LOG(LogGameEngine, Log, "Engine shutdown complete\n");
		ULog::Get().Shutdown();
	}

	bool UGameEngine::Init_Internal(eastl::shared_ptr<UGameWindow> inGameWindow)
	{
		m_gameWindow = inGameWindow;

		// Init renderer
		m_renderer = eastl::make_shared<URenderer>();
		if (!m_renderer->Init(*m_gameWindow))
		{
			return false;
		}

		// Init the physics world
		m_physicsWorld = eastl::make_shared<UPhysicsWorld>(nullptr);
		if (!m_physicsWorld)
		{
			return false;
		}

		// Init networking manager
		if (!m_networkManager.Init())
		{
			return false;
		}

		// Start the FrameTimer
		m_frameTimer = eastl::make_shared<UFrameTimer>();
		m_frameTimer->Start();

		// Create the GameInstance
		m_gameInstance = eastl::make_shared<UGameInstance>();
		m_gameInstance->OnStartup();

		return true;
	}

	void UGameEngine::PreTick_Internal(float inDeltaTime)
	{
		DrawOnScreeDebugText(inDeltaTime);
	}

	// TEMP: Remove once we have proper loading system. For now, creates one GameWorld with 2 Layers, Default_Layer and Geometry_Layer, to test
	void UGameEngine::InitializeEngineContext()
	{
		SControlScheme& renderScheme = SControlScheme("RenderDebug")
			.RegisterEvent("NormalView", '0')
			.RegisterEvent("LightAccumulationView", '1')
			.RegisterEvent("DiffuseView", '2')
			.RegisterEvent("SpecularView", '3')
			.RegisterEvent("NormalsView", '4')
			.RegisterEvent("DepthView", '5')
			.RegisterEvent("DebugView", '6')
			.RegisterEvent("ToggleHUD", '7')
			.Finalize(true);

		SControlScheme("CameraDebug")
			.RegisterAxis("Vertical", VK_SPACE, VK_SHIFT)
			.RegisterAxis("Horizontal", 'D', 'A')
			.RegisterAxis("Forward", 'W', 'S')
			.RegisterAxis("LookX", EInputAxis::IA_MouseX)
			.RegisterAxis("LookY", EInputAxis::IA_MouseY)
			.RegisterEvent("RightClick", VK_RBUTTON)
			.RegisterEvent("Reset", 'R')
			.Finalize(false);

		SControlScheme("DemoCharacter")
			.RegisterAxis("Vertical", VK_SPACE, VK_SHIFT)
			.RegisterAxis("Horizontal", 'D', 'A')
			.RegisterAxis("Forward", 'W', 'S')
			.RegisterAxis("LookX", EInputAxis::IA_MouseX)
			.RegisterAxis("LookY", EInputAxis::IA_MouseY)
			.RegisterEvent("Shoot", VK_LBUTTON)
			.RegisterEvent("DebugLine", VK_RBUTTON)
			.RegisterEvent("MouseMode", 'M')
			.Finalize(true);

		SControlScheme& debugScheme = SControlScheme("Debug")
			.RegisterEvent("ReloadWorld", 'T')
			.Finalize(true);

		renderScheme.BindEvent<&OnDisableGBufferVisualization>("NormalView", EInputEvent::IE_KeyDown);
		renderScheme.BindEvent<&OnEnableLightAccumulation>("LightAccumulationView", EInputEvent::IE_KeyDown);
		renderScheme.BindEvent<&OnEnableDiffuse>("DiffuseView", EInputEvent::IE_KeyDown);
		renderScheme.BindEvent<&OnEnableSpecular>("SpecularView", EInputEvent::IE_KeyDown);
		renderScheme.BindEvent<&OnEnableNormals>("NormalsView", EInputEvent::IE_KeyDown);
		renderScheme.BindEvent<&OnEnableDepth>("DepthView", EInputEvent::IE_KeyDown);
		renderScheme.BindEvent<&OnEnableDebugPrimitives>("DebugView", EInputEvent::IE_KeyDown);
		renderScheme.BindEvent<&OnToggleHUD>("ToggleHUD", EInputEvent::IE_KeyDown);

		debugScheme.BindEvent<UBaseEngine, &UBaseEngine::ReloadAllWorlds>("ReloadWorld", EInputEvent::IE_KeyDown, this);
	}

	void UGameEngine::DrawOnScreeDebugText(float inFrameTime)
	{
		m_renderer->DrawOnScreenText(eastl::string("FPS: ").append(eastl::to_string(1.0f / inFrameTime)), 25, 25);
		m_renderer->DrawOnScreenText(eastl::string("Num Worlds: ").append(eastl::to_string(m_worlds.size())), 25, 50);

		for (const auto& currentWorld : m_worlds)
		{
			eastl::string worldInfoString;

			worldInfoString.sprintf("------%s: %d entities", currentWorld->GetWorldName().c_str(), currentWorld->GetEntityCount());

			m_renderer->DrawOnScreenText(worldInfoString, 25, 75);
		}
	}

}
