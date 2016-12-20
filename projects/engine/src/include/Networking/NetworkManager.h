#pragma once

#include <EASTL/shared_ptr.h>
#include <yojimbo/yojimbo.h>

#include "Networking/Network.h"
#include "Networking/NetworkClient.h"
#include "Networking/NetworkPlayer.h"
#include "Networking/NetworkServer.h"

namespace MAD
{
	class UNetworkManager
	{
	public:
		UNetworkManager();

		bool Init();
		void PreTick();
		void PostTick();
		void Shutdown();

		ENetMode GetNetMode() const { return m_netMode; }

		eastl::weak_ptr<ONetworkPlayer> GetLocalPlayer() const { return m_client ? m_client->GetLocalPlayer() : eastl::weak_ptr<ONetworkPlayer>(); }
		size_t GetNumPlayers() const;

		void DestroyNetworkObject(UObject& inObject);

		void SendNetworkEvent(EEventTarget inEventTarget, EEventTypes inEventType, UObject& inTargetObject, void* inEventData, size_t inEventSize, NetworkPlayerID_t inTargetPlayer = InvalidPlayerID);

		template <typename ObjectType>
		eastl::shared_ptr<ObjectType> SpawnNetworkObject(const UObject* inNetOwnerContext)
		{
			static_assert(eastl::is_base_of<UObject, ObjectType>::value, "Spawned ObjectType must be of type UObject or more derived");

			MAD_ASSERT_DESC(m_netMode != ENetMode::Client, "Warning: Cannot spawn networked entities on client!\n");
			
			if (m_netMode == ENetMode::Client) return nullptr;

			MAD_ASSERT_DESC(inNetOwnerContext != nullptr, "Net context cannot be null");
			MAD_ASSERT_DESC(inNetOwnerContext->IsNetworkSpawned(), "Net context must be a network-spawned object to obtain its net owner");

			if (inNetOwnerContext == nullptr) return nullptr;

			const TTypeInfo* objectTypeInfo = ObjectType::StaticClass();
			return eastl::static_pointer_cast<ObjectType>(m_server->SpawnNetworkObject(*objectTypeInfo, *inNetOwnerContext->GetNetOwner()));
		}

		template <typename ObjectType>
		eastl::shared_ptr<ObjectType> SpawnNetworkObject(const TTypeInfo& inTypeInfo, const UObject* inNetOwnerContext)
		{
			static_assert(eastl::is_base_of<UObject, ObjectType>::value, "Spawned ObjectType must be of type UObject or more derived");
			MAD_ASSERT_DESC(IsA<ObjectType>(inTypeInfo), "Given type info must be a derived class of UObject");

			MAD_ASSERT_DESC(m_netMode != ENetMode::Client, "Warning: Cannot spawn networked entities on client!\n");

			if (m_netMode == ENetMode::Client) return nullptr;

			MAD_ASSERT_DESC(inNetOwnerContext != nullptr, "Net context cannot be null");
			MAD_ASSERT_DESC(inNetOwnerContext->IsNetworkSpawned(), "Net context must be a network-spawned object to obtain its net owner");

			if (inNetOwnerContext == nullptr) return nullptr;

			return eastl::static_pointer_cast<ObjectType>(m_server->SpawnNetworkObject(inTypeInfo, *inNetOwnerContext->GetNetOwner()));
		}

		template <typename EntityType>
		eastl::shared_ptr<EntityType> SpawnNetworkEntity(const UObject* inNetOwnerContext, OGameWorld* inOwningGameWorld, const eastl::string& inWorldLayer)
		{
			static_assert(eastl::is_base_of<AEntity, EntityType>::value, "Spawned EntityType must be of type AEntity or more derived");
			MAD_ASSERT_DESC(m_netMode != ENetMode::Client, "Warning: Cannot spawn networked entities on client!\n");

			if (m_netMode == ENetMode::Client) return nullptr;

			MAD_ASSERT_DESC(inNetOwnerContext != nullptr, "Net context cannot be null");
			MAD_ASSERT_DESC(inNetOwnerContext->IsNetworkSpawned(), "Net context must be a network-spawned object to obtain its net owner");

			if (inNetOwnerContext == nullptr) return nullptr;

			const TTypeInfo* entityTypeInfo = EntityType::StaticClass();
			return eastl::static_pointer_cast<EntityType>(m_server->SpawnNetworkEntity(*entityTypeInfo, *inNetOwnerContext->GetNetOwner(), inOwningGameWorld, inWorldLayer));
		}

		template <typename EntityType>
		eastl::shared_ptr<EntityType> SpawnNetworkEntity(const TTypeInfo& inTypeInfo, const UObject* inNetOwnerContext, OGameWorld* inOwningGameWorld, const eastl::string& inWorldLayer)
		{
			static_assert(eastl::is_base_of<AEntity, EntityType>::value, "Spawned EntityType must be of type AEntity or more derived");
			MAD_ASSERT_DESC(IsA<EntityType>(inTypeInfo), "Given type info must be a derived class of AEntity");
			MAD_ASSERT_DESC(m_netMode != ENetMode::Client, "Warning: Cannot spawn networked entities on client!\n");

			if (m_netMode == ENetMode::Client) return nullptr;

			MAD_ASSERT_DESC(inNetOwnerContext != nullptr, "Net context cannot be null");
			MAD_ASSERT_DESC(inNetOwnerContext->IsNetworkSpawned(), "Net context must be a network-spawned object to obtain its net owner");

			if (inNetOwnerContext == nullptr) return nullptr;

			return eastl::static_pointer_cast<EntityType>(m_server->SpawnNetworkEntity(inTypeInfo, *inNetOwnerContext->GetNetOwner(), inOwningGameWorld, inWorldLayer));
		}

	private:
		friend class UNetworkClient;
		friend class UNetworkServer;

		ENetMode m_netMode;
		yojimbo::ClientServerConfig m_config;

		eastl::unique_ptr<UNetworkServer> m_server;
		eastl::unique_ptr<UNetworkClient> m_client;

		bool StartServer(int port);
		bool ConnectToServer(const yojimbo::Address& inServerAddress);
	};
}
