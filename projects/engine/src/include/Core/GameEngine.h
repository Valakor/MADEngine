#pragma once

#include <EASTL/shared_ptr.h>
#include <EASTL/weak_ptr.h>
#include <EASTL/string.h>
#include <EASTL/vector.h>
#include <EASTL/type_traits.h>

#include "Networking/NetworkManager.h"

namespace MAD
{
	class TTypeInfo;

	struct SCmdLine
	{
		static const eastl::string& Get() { return mCmdLine; }

		static void SetCmdLine(const eastl::string& inCmdLine) { mCmdLine = eastl::move(inCmdLine); }

	private:
		static eastl::string mCmdLine;
	};

	extern class UGameEngine* gEngine;

	class UGameEngine
	{
	public:
		UGameEngine();
		~UGameEngine();

		bool Init(const eastl::string& inGameName, int inWindowWidth, int inWindowHeight);
		void Run();
		void Stop();

		template <typename WorldType>
		eastl::weak_ptr<WorldType> SpawnGameWorld(const eastl::string& inWorldName, const eastl::string& inWorldRelativePath);

		// Since the TTypeInfo doesn't explicitly give us class information, the CommonAncetorWorldType
		// is used as a "I know it is at least derived from this"
		template <typename CommonAncestorWorldType>
		eastl::weak_ptr<CommonAncestorWorldType> SpawnGameWorld(const TTypeInfo& inTypeInfo, const eastl::string& inWorldName, const eastl::string& inWorldRelativePath);

		bool IsSimulating() const { return m_isSimulating; }
		float GetDeltaTime() const { return static_cast<float>(TARGET_DELTA_TIME); }
		float GetFrameTime() const { return static_cast<float>(m_frameTime); }
		float GetGameTime() const { return static_cast<float>(m_gameTime); }
		double GetGameTimeDouble() const { return m_gameTime; }
		uint32_t GetGameTick() const { return m_gameTick; }

		eastl::shared_ptr<class OGameWorld> GetWorld(const eastl::string& inWorldName);
		eastl::shared_ptr<class OGameWorld> GetWorld(size_t inIndex);

		class URenderer& GetRenderer() const { return *m_renderer; }
		class UGameWindow& GetWindow() const { return *m_gameWindow; }
		class UPhysicsWorld& GetPhysicsWorld() const { return *m_physicsWorld; }
		UNetworkManager& GetNetworkManager() { return m_networkManager; }
	private:
		int32_t GetWorldIndex(const eastl::string& inWorldName) const;

		// TODO Reloading world doesn't totally work with the network because we don't respawn the player again
		bool ReloadWorld(size_t inWorldIndex);
		bool ReloadWorld(const eastl::string& inWorldName);
		void ReloadAllWorlds();

		void ExecuteEngineTests();

		void TEMPInitializeGameContext();
		void TEMPSerializeObject();
		void TEMPDrawOnScreenDebugText(double inFrameTime);
	private:
		const int MAX_SIMULATION_STEPS = 10;
		//const double TARGET_DELTA_TIME = 0.016666666666666666; // 60 FPS
		const double TARGET_DELTA_TIME = 0.05; // 20 FPS

		void Tick();

		bool m_bContinue;
		bool m_isSimulating;

		uint32_t m_gameTick; // Enough bits for 19,884 hours of gameplay @ 60Hz simulation
		double m_gameTime;
		double m_frameTime;
		double m_frameAccumulator;

		eastl::vector<eastl::shared_ptr<class OGameWorld>> m_worlds;
		eastl::shared_ptr<class UFrameTimer> m_frameTimer;
		eastl::shared_ptr<class UGameInstance> m_gameInstance;
		eastl::shared_ptr<class UGameWindow> m_gameWindow;
		eastl::shared_ptr<class UPhysicsWorld> m_physicsWorld;
		eastl::shared_ptr<class URenderer> m_renderer;
		UNetworkManager m_networkManager;
	};

	template <typename WorldType>
	eastl::weak_ptr<WorldType> UGameEngine::SpawnGameWorld(const eastl::string& inWorldName, const eastl::string& inWorldRelativePath)
	{
		static_assert(eastl::is_base_of<OGameWorld, WorldType>::value, "Error: You may only spawn game worlds that are of type UGameWorld or more derived");
		return SpawnGameWorld<WorldType>(*WorldType::StaticClass(), inWorldName, inWorldRelativePath);
	}

	template <typename CommonAncestorWorldType>
	eastl::weak_ptr<CommonAncestorWorldType> UGameEngine::SpawnGameWorld(const TTypeInfo& inTypeInfo, const eastl::string& inWorldName, const eastl::string& inWorldRelativePath)
	{
		static_assert(eastl::is_base_of<OGameWorld, CommonAncestorWorldType>::value, "Error: You may only spawn game worlds that are of type UGameWorld or more derived");

		// Pass nullptr to the owning UGameWorld because a UGameWorld isn't contained within another UGameWorld
		eastl::shared_ptr<CommonAncestorWorldType> newWorld = CreateDefaultObject<CommonAncestorWorldType>(inTypeInfo, nullptr);;

		newWorld->SetWorldName(inWorldName);
		newWorld->SetWorldRelativePath(inWorldRelativePath);

		m_worlds.emplace_back(newWorld);

		return newWorld;
	}
}
