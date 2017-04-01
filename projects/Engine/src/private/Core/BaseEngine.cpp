#include "Core/GameEngine.h"

#include <EASTL/algorithm.h>

#include "Core/FrameTimer.h"
#include "Core/GameInput.h"
#include "Core/GameInstance.h"
#include "Core/GameWindow.h"
#include "Core/GameWorld.h"
#include "Core/Pipeline/GameWorldLoader.h"
#include "Core/PhysicsWorld.h"
#include "Misc/AssetCache.h"
#include "Misc/Parse.h"
#include "Rendering/Renderer.h"

#include "Core/Character.h"
#include "Core/CameraComponent.h"
#include "Core/MeshComponent.h"
#include "Core/LightComponent.h"
#include "Core/DirectionalLightComponent.h"
#include "Core/PointLightComponent.h"
#include "Core/DebugTransformComponent.h"
#include "Rendering/ParticleSystem/ParticleSystemComponent.h"

#include "Editor/SceneCameraCharacter.h"

#include "Networking/NetworkState.h"

// TESTING
#include "Testing/TestCharacters.h"
#include "Testing/TestComponents.h"
#include "Testing/EntityTestingModule.h"

#include "Rendering/FontFamily.h"

#include <stdlib.h>
#include <time.h>

using eastl::string;

namespace MAD
{
	string SCmdLine::mCmdLine;

	DECLARE_LOG_CATEGORY(LogBaseEngine);

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

	namespace
	{
		// TODO Clean this up somehow to make it less manual
		void RegisterAllTypeInfos()
		{
			UObject::StaticClass();
			OGameWorld::StaticClass();

			AEntity::StaticClass();
			ACharacter::StaticClass();
			ASceneCameraCharacter::StaticClass();

			UComponent::StaticClass();
			CCameraComponent::StaticClass();
			CMeshComponent::StaticClass();
			CLightComponent::StaticClass();
			CDirectionalLightComponent::StaticClass();
			CPointLightComponent::StaticClass();
			CMoveComponent::StaticClass();
			CDebugTransformComponent::StaticClass();
			CParticleSystemComponent::StaticClass();

			Test::RegisterEntityTypes();
			Test::RegisterComponentTypes();
		}
	}

	const eastl::string UBaseEngine::s_defaultLevelPath = "engine\\worlds\\default_world.json";

	UBaseEngine* gEngine = nullptr;

	UBaseEngine::UBaseEngine()
		: m_bIsInitialized(false)
		, m_bContinue(true)
		, m_isSimulating(false)
		, m_gameTick(0)
		, m_gameTime(0.0)
		, m_frameTime(0.0)
		, m_frameAccumulator(0.0) { }

	bool UBaseEngine::Init(eastl::shared_ptr<UGameWindow> inGameWindow)
	{
		srand(time(nullptr));

		RegisterAllTypeInfos();
		TTypeInfo::DumpTypeInfo();

		UGameWindow::SetWorkingDirectory();
		UAssetCache::SetAssetRoot(UGameWindow::GetWorkingDirectory() + "\\assets\\");

		SCmdLine::SetCmdLine(UGameWindow::GetNativeCommandline());
		ULog::Get().Init();

		LOG(LogBaseEngine, Log, "Engine initialization begin...\n");
		LOG(LogBaseEngine, Log, "Commandline: %s\n", SCmdLine::Get().c_str());

		// Set global engine pointer
		gEngine = this;

		if (!Init_Internal(inGameWindow))
		{
			return false;
		}

		InitializeEngineContext();

		eastl::string levelPath = s_defaultLevelPath;

		SParse::Get(SCmdLine::Get(), "-Level=", levelPath);

		// Load the world after setting up control schemes. (We could probably define those in the world file or something as well)
		UGameWorldLoader loader;

		loader.LoadWorld(levelPath);

		m_bIsInitialized = true;

		LOG(LogBaseEngine, Log, "Engine initialization successful\n");
		return true;
	}

	void UBaseEngine::Stop()
	{
		LOG(LogBaseEngine, Log, "Engine stopping...\n");
		m_bContinue = false;
	}

	eastl::shared_ptr<OGameWorld> UBaseEngine::GetWorld(const string& inWorldName)
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

	eastl::shared_ptr<OGameWorld> UBaseEngine::GetWorld(size_t inIndex)
	{
		if (inIndex >= m_worlds.size())
		{
			LOG(LogBaseEngine, Warning, "World with index '%i' does not exist\n", inIndex);
			return nullptr;
		}

		return m_worlds[inIndex];
	}

	int32_t UBaseEngine::GetWorldIndex(const eastl::string& inWorldName) const
	{
		const size_t numWorlds = m_worlds.size();
		for (size_t i = 0; i < numWorlds; ++i)
		{
			if (m_worlds[i]->GetWorldName() == inWorldName)
			{
				return static_cast<int32_t>(i);
			}
		}

		return -1;
	}

	bool UBaseEngine::ReloadWorld(size_t inWorldIndex)
	{
		if (inWorldIndex >= m_worlds.size())
		{
			return false;
		}

		// Swap and pop the last world with the index specified
		const eastl::string targetWorldRelativePath = m_worlds[inWorldIndex]->GetWorldRelativePath();

		m_worlds[inWorldIndex] = m_worlds.back();

		m_worlds.pop_back();

		UGameWorldLoader gameWorldLoader;

		return gameWorldLoader.LoadWorld(targetWorldRelativePath);
	}

	bool UBaseEngine::ReloadWorld(const eastl::string& inWorldName)
	{
		if (inWorldName.empty())
		{
			return false;
		}

		int32_t targetWorldIndex = GetWorldIndex(inWorldName);

		if (targetWorldIndex == -1)
		{
			return false;
		}

		return ReloadWorld(static_cast<size_t>(targetWorldIndex));
	}

	void UBaseEngine::ReloadAllWorlds()
	{
		const size_t numWorlds = m_worlds.size();

		for (size_t i = 0; i < numWorlds; ++i)
		{
			ReloadWorld(i);
		}
	}

	void UBaseEngine::ExecuteEngineTests()
	{
		// Assumes that the default world loaded in correctly
		if (!m_worlds.empty())
		{
			eastl::shared_ptr<OGameWorld> defaultWorld = m_worlds[0];

			MAD_ASSERT_DESC(Test::TestEntityModule(*defaultWorld), "Error: The entity testing module didn't pass all of the tests!");
		}
	}

	void UBaseEngine::TEMPSerializeObject()
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

	void UBaseEngine::Tick()
	{
		auto now = m_frameTimer->TimeSinceStart();

		m_frameTime = now - m_gameTime;
		m_gameTime = now;

		m_frameAccumulator += m_frameTime;

		int steps = eastl::min(static_cast<int>(m_frameAccumulator / TARGET_DELTA_TIME), MAX_SIMULATION_STEPS);
		m_frameAccumulator -= steps * TARGET_DELTA_TIME;

		PreTick_Internal(m_frameTime);
		
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
		m_renderer->Frame(framePercent, m_frameTime);

		PostTick_Internal(m_frameTime);
	}
}
