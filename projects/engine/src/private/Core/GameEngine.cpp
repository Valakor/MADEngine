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

// TESTING
#include "Core/Character.h"
#include "Core/CameraComponent.h"
#include "Core/MeshComponent.h"
#include "Core/LightComponent.h"
#include "Core/DirectionalLightComponent.h"
#include "Core/PointLightComponent.h"

#include "Networking/NetworkState.h"

#include "Testing/TestCharacters.h"
#include "Testing/EntityTestingModule.h"

using eastl::string;

namespace MAD
{
	string SCmdLine::mCmdLine;

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
	}

	namespace
	{
		// TODO Clean this up somehow to make it less manual
		void RegisterAllTypeInfos()
		{
			UObject::StaticClass();
			OGameWorld::StaticClass();

			AEntity::StaticClass();
			ACharacter::StaticClass();

			UComponent::StaticClass();
			CCameraComponent::StaticClass();
			CMeshComponent::StaticClass();
			CLightComponent::StaticClass();
			CDirectionalLightComponent::StaticClass();
			CPointLightComponent::StaticClass();
			CMoveComponent::StaticClass();

			Test::APointLightBullet::StaticClass();
			Test::ADemoCharacter::StaticClass();

			Test::CTimedDeathComponent::StaticClass();
			Test::CPointLightBulletComponent::StaticClass();
			Test::CDemoCharacterController::StaticClass();
			Test::CSinMoveComponent::StaticClass();
		}
	}

	UGameEngine* gEngine = nullptr;

	UGameEngine::UGameEngine()
		: m_bContinue(true)
		, m_isSimulating(false)
		, m_gameTick(0)
		, m_gameTime(0.0)
		, m_frameTime(0.0)
		, m_frameAccumulator(0.0) { }

	bool UGameEngine::Init(const string& inGameName, int inWindowWidth, int inWindowHeight)
	{
		RegisterAllTypeInfos();
		TTypeInfo::DumpTypeInfo();

		UGameWindow::SetWorkingDirectory();
		UAssetCache::SetAssetRoot(UGameWindow::GetWorkingDirectory() + "\\assets\\");

		SCmdLine::SetCmdLine(UGameWindow::GetNativeCommandline());
		ULog::Get().Init();

		LOG(LogGameEngine, Log, "Engine initialization begin...\n");
		LOG(LogGameEngine, Log, "Commandline: %s\n", SCmdLine::Get().c_str());

		// Create a window
		m_gameWindow = eastl::make_shared<UGameWindow>();
		if (!UGameWindow::CreateGameWindow(inGameName, inWindowWidth, inWindowHeight, *m_gameWindow))
		{
			return false;
		}

		// Set global engine ptr
		gEngine = this;

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

		LOG(LogGameEngine, Log, "Engine initialization successful\n");
		return true;
	}

	void UGameEngine::Run()
	{
		// In the future, update defaults by configuration file
		TEMPInitializeGameContext();

		//TEMPSerializeObject();

		TEMPTestTransformHierarchy();

		while (m_bContinue)
		{
			Tick();
		}

		m_gameWindow->CaptureCursor(false);
	}

	void UGameEngine::Stop()
	{
		LOG(LogGameEngine, Log, "Engine stopping...\n");
		m_bContinue = false;
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

	eastl::shared_ptr<OGameWorld> UGameEngine::GetWorld(const string& inWorldName)
	{
		eastl::shared_ptr<OGameWorld> world;

		for (auto w : m_worlds)
		{
			if (w->GetWorldName() == inWorldName)
			{
				return w;
			}
		}

		return nullptr;
	}

	eastl::shared_ptr<OGameWorld> UGameEngine::GetWorld(size_t inIndex)
	{
		if (inIndex >= m_worlds.size())
		{
			LOG(LogGameEngine, Warning, "World with index '%i' does not exist\n", inIndex);
			return nullptr;
		}

		return m_worlds[inIndex];
	}

	// TEMP: Remove once we have proper loading system. For now, creates one GameWorld with 2 Layers, Default_Layer and Geometry_Layer, to test
	void UGameEngine::TEMPInitializeGameContext()
	{
		SControlScheme& renderScheme = SControlScheme("RenderDebug")
			.RegisterEvent("NormalView", '0')
			.RegisterEvent("LightAccumulationView", '1')
			.RegisterEvent("DiffuseView", '2')
			.RegisterEvent("SpecularView", '3')
			.RegisterEvent("NormalsView", '4')
			.RegisterEvent("DepthView", '5')
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

		debugScheme.BindEvent<UGameEngine, &UGameEngine::TEMPReloadWorld>("ReloadWorld", EInputEvent::IE_KeyDown, this);

		// Load the world _after_ setting up control schemes. (We could probably define those in the world file or something as well)
		TEMPReloadWorld();
	}

	void UGameEngine::TEMPReloadWorld()
	{
		m_worlds.clear();

		UGameWorldLoader loader;
		loader.LoadWorld("engine\\worlds\\default_world.json");
		//loader.LoadWorld("engine\\worlds\\sponza_world.json");
	}

	void UGameEngine::TEMPSerializeObject()
	{
		eastl::shared_ptr<OGameWorld> defaultWorld = m_worlds[0];

		MAD_ASSERT_DESC(defaultWorld != nullptr, "Error: The first default world needs to be created!\n");

		auto networkedEntity = defaultWorld->SpawnEntity<MAD::Test::ANetworkedEntity>();

		if (networkedEntity)
		{
			eastl::vector<uint8_t> serializationBuffer;
			eastl::vector<uint8_t> deserializationBuffer;

			networkedEntity->m_networkedFloat = 5.0f;
			networkedEntity->m_networkedUInt = 1337;

			// Create a network state to view this networked entity
			UNetworkState activeNetworkState;

			activeNetworkState.TargetObject(networkedEntity.get(), true);

			networkedEntity->m_networkedFloat = -1.0f;
			networkedEntity->m_networkedUInt = 0xdeadbeef;

			// Simulate serialization of the entity data
			activeNetworkState.SerializeState(serializationBuffer, false);

			deserializationBuffer = serializationBuffer;

			activeNetworkState.SerializeState(deserializationBuffer, true);
		}
	}

	void UGameEngine::TEMPTestTransformHierarchy()
	{
		// Assumes that the default world loaded in correctly
		if (!m_worlds.empty())
		{
			eastl::shared_ptr<OGameWorld> defaultWorld = m_worlds[0];

			MAD_ASSERT_DESC(Test::TestEntityModule(*defaultWorld), "Error: The entity testing module didn't pass all of the tests!");
		}
	}

	void UGameEngine::Tick()
	{
		auto now = m_frameTimer->TimeSinceStart();
		auto frameTime = now - m_gameTime;
		m_gameTime = now;

		m_frameAccumulator += frameTime;

		int steps = eastl::min(static_cast<int>(m_frameAccumulator / TARGET_DELTA_TIME), MAX_SIMULATION_STEPS);
		m_frameAccumulator -= steps * TARGET_DELTA_TIME;

		while (steps > 0)
		{
			// Update current game tick
			m_gameTick++;
			MAD_ASSERT_DESC(m_gameTick != 0, "Game tick overflow detected");

			// Clear the old draw items
			m_renderer->ClearRenderItems();

			// Recieve from the network
			m_networkManager.PreTick();

			// Tick native message queue
			UGameWindow::PumpMessageQueue();

			// Tick input
			UGameInput::Get().Tick();

			// Moved simulating flag to engine because we want all worlds to only perform post simulation tasks
			// once all worlds have had its chance to simulate
			m_isSimulating = true;

			// Tick the pre-physics components of all Worlds
			for (auto& currentWorld : m_worlds)
			{
				currentWorld->UpdatePrePhysics(TARGET_DELTA_TIME);
			}

			//// Update the physics world
			m_physicsWorld->SimulatePhysics();

			//// Tick the post-physics components of all Worlds
			for (auto& currentWorld : m_worlds)
			{
				currentWorld->UpdatePostPhysics(TARGET_DELTA_TIME);
			}

			m_isSimulating = false;

			// Send to the network
			m_networkManager.PostTick();

			// Perform clean up on each of the worlds before we perform any updating (i.e in case entities are pending for kill)
			for (auto& currentWorld : m_worlds)
			{
				currentWorld->CleanupEntities();
			}

			steps--;
		}

		// How far we are along in this frame
		float framePercent = static_cast<float>(m_frameAccumulator / TARGET_DELTA_TIME);

		// Tick renderer
		m_renderer->Frame(framePercent);
	}
}
