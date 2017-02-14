#include "Editor/EditorEngine.h"

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

namespace MAD
{
	DECLARE_LOG_CATEGORY(LogEditorEngine);

	UEditorEngine::~UEditorEngine()
	{
		m_worlds.clear();

		m_gameInstance->OnShutdown();
		m_gameInstance = nullptr;

		m_renderer->Shutdown();
		m_renderer = nullptr;

		m_gameWindow = nullptr;

		LOG(LogEditorEngine, Log, "Engine shutdown complete\n");
		ULog::Get().Shutdown();
	}

	bool UEditorEngine::Init_Internal(eastl::shared_ptr<class UGameWindow> inGameWindow)
	{
		if (!inGameWindow || (inGameWindow && !inGameWindow->GetHWnd()))
		{
			return false;
		}

		// The scene editor window will already be created from the host editor application
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
		/*if (!m_networkManager.Init())
		{
			return false;
		}*/

		// Start the FrameTimer
		m_frameTimer = eastl::make_shared<UFrameTimer>();
		m_frameTimer->Start();

		// Create the GameInstance
		m_gameInstance = eastl::make_shared<UGameInstance>();
		m_gameInstance->OnStartup();

		return true;
	}

	void UEditorEngine::InitializeEngineContext()
	{
		SControlScheme("CameraDebug")
			.RegisterAxis("Vertical", VK_SPACE, VK_SHIFT)
			.RegisterAxis("Horizontal", 'D', 'A')
			.RegisterAxis("Forward", 'W', 'S')
			.RegisterAxis("LookX", EInputAxis::IA_MouseX)
			.RegisterAxis("LookY", EInputAxis::IA_MouseY)
			.RegisterEvent("RightClick", VK_RBUTTON)
			.RegisterEvent("Reset", 'R')
			.Finalize(true);

		UGameWorldLoader loader;

		loader.LoadWorld("engine\\worlds\\new_world.json");
	}

}