#include "Core/GameEngine.h"

#include <EASTL/algorithm.h>

#include "Core/FrameTimer.h"
#include "Core/GameInput.h"
#include "Core/GameInstance.h"
#include "Core/GameWindow.h"
#include "Core/GameWorld.h"
#include "Core/PhysicsWorld.h"
#include "Misc/AssetCache.h"
#include "Rendering/Renderer.h"

// TESTING
#include "Core/Character.h"
#include "Core/TestCharacters.h"
#include "Rendering/Mesh.h"

using eastl::string;

namespace MAD
{
	string SCmdLine::mCmdLine;

	DECLARE_LOG_CATEGORY(LogGameEngine);

	UGameEngine* gEngine = nullptr;

	UGameEngine::UGameEngine()
		: bContinue(true)
		, m_isSimulating(false)
	    , mGameTime(0.0)
	    , mFrameTime(0.0)
	    , mFrameAccumulator(0.0) { }

	bool UGameEngine::Init(const string& inGameName, int inWindowWidth, int inWindowHeight)
	{
		UGameWindow::SetWorkingDirectory();
		UAssetCache::SetAssetRoot(UGameWindow::GetWorkingDirectory() + "\\assets\\");

		SCmdLine::SetCmdLine(UGameWindow::GetNativeCommandline());
		ULog::Get().Init();

		LOG(LogGameEngine, Log, "Engine initialization begin...\n");
		LOG(LogGameEngine, Log, "Commandline: %s\n", SCmdLine::Get().c_str());

		// Create a window
		mGameWindow = eastl::make_shared<UGameWindow>();
		if (!UGameWindow::CreateGameWindow(inGameName, inWindowWidth, inWindowHeight, *mGameWindow))
		{
			return false;
		}

		// Set global engine ptr
		gEngine = this;

		// Init renderer
		mRenderer = eastl::make_shared<URenderer>();
		if (!mRenderer->Init(*mGameWindow))
		{
			return false;
		}

		// TODO testing
		auto cube2 = UAssetCache::Load<UMesh>("engine\\meshes\\primitives\\cube.obj");

		// Init the physics world
		m_physicsWorld = eastl::make_shared<UPhysicsWorld>(nullptr);
		if (!m_physicsWorld)
		{
			return false;
		}

		// Start the FrameTimer
		mFrameTimer = eastl::make_shared<UFrameTimer>();
		mFrameTimer->Start();

		// Create the GameInstance
		mGameInstance = eastl::make_shared<UGameInstance>();
		mGameInstance->OnStartup();

		LOG(LogGameEngine, Log, "Engine initialization successful\n");
		return true;
	}

	void UGameEngine::Run()
	{
		// In the future, update defaults by configuration file
		TEMPInitializeGameContext();

		while (bContinue)
		{
			Tick();
		}

		mGameWindow->CaptureCursor(false);
	}

	void UGameEngine::Stop()
	{
		LOG(LogGameEngine, Log, "Engine stopping...\n");
		bContinue = false;
	}

	UGameEngine::~UGameEngine()
	{
		mGameInstance->OnShutdown();
		mGameInstance = nullptr;

		mRenderer->Shutdown();
		mRenderer = nullptr;

		mGameWindow = nullptr;

		LOG(LogGameEngine, Log, "Engine shutdown complete\n");
		ULog::Get().Shutdown();
	}

	// TEMP: Remove once we have proper loading system. For now, creates one GameWorld with 2 Layers, Default_Layer and Geometry_Layer, to test
	void UGameEngine::TEMPInitializeGameContext()
	{
		eastl::weak_ptr<OGameWorld> initialGameWorld = SpawnGameWorld<OGameWorld>("Gameplay_World");

		initialGameWorld.lock()->SpawnEntity<ACharacter>();
	}

	void UGameEngine::Tick()
	{
		auto now = mFrameTimer->TimeSinceStart();
		auto frameTime = now - mGameTime;
		mGameTime = now;

		mFrameAccumulator += frameTime;

		int steps = eastl::min(static_cast<int>(floor(mFrameAccumulator / TARGET_DELTA_TIME)), MAX_SIMULATION_STEPS);
		mFrameAccumulator -= steps * TARGET_DELTA_TIME;

		while (steps > 0)
		{
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
				currentWorld->UpdatePrePhysics(static_cast<float>(TARGET_DELTA_TIME));
			}

			//// Update the physics world
			m_physicsWorld->SimulatePhysics();


			//// Tick the post-physics components of all Worlds
			for (auto& currentWorld : m_worlds)
			{
				currentWorld->UpdatePostPhysics(static_cast<float>(TARGET_DELTA_TIME));
			}

			m_isSimulating = false;

			// Perform clean up on each of the worlds before we perform any updating (i.e in case entities are pending for kill)
			for (auto& currentWorld : m_worlds)
			{
				currentWorld->CleanupEntities();
			}

			steps--;
		}

		// How far we are along in this frame
		float framePercent = static_cast<float>(mFrameAccumulator / TARGET_DELTA_TIME);

		// Tick renderer
		mRenderer->Frame(framePercent);
	}
}
