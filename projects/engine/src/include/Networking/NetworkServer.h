#pragma once

#include <EASTL/hash_map.h>
#include <EASTL/unique_ptr.h>
#include <yojimbo/yojimbo.h>

#include "Networking/NetworkTypes.h"
#include "Networking/NetworkTransport.h"
#include "Networking/NetworkPlayer.h"
#include "Networking/NetworkObjectView.h"
#include "Networking/NetworkState.h"

namespace MAD
{
	class UNetworkServer : public yojimbo::Server
	{
	public:
		explicit UNetworkServer(class UNetworkManager& inNetworkManager, eastl::unique_ptr<UNetworkTransport> inServerTransport, double inCurrentGameTime, const yojimbo::ClientServerConfig& inConfig = yojimbo::ClientServerConfig());

		void Tick(float inGameTime);

		size_t GetNumConnectedPlayers() const { return m_players.size(); }
		eastl::weak_ptr<ONetworkPlayer> GetPlayerByID(NetworkPlayerID inID) const;

		eastl::shared_ptr<UObject> GetNetworkObject(SNetworkID inNetworkID) const;

		template <typename ObjectType>
		eastl::shared_ptr<ObjectType> SpawnNetworkObject()
		{
			static_assert(eastl::is_base_of<UObject, ObjectType>::value, "Spawned ObjectType must be of type UObject or more derived");
			const TTypeInfo* objectTypeInfo = ObjectType::StaticClass();
			return eastl::static_pointer_cast<ObjectType>(SpawnNetworkObject_Internal(*objectTypeInfo));
		}

		template <typename ObjectType>
		eastl::shared_ptr<ObjectType> SpawnNetworkObject(const TTypeInfo& inTypeInfo)
		{
			static_assert(eastl::is_base_of<UObject, ObjectType>::value, "Spawned ObjectType must be of type UObject or more derived");
			MAD_ASSERT_DESC(IsA<ObjectType>(inTypeInfo), "Given type info must be a derived class of UObject");
			return eastl::static_pointer_cast<ObjectType>(SpawnNetworkObject_Internal(inTypeInfo));
		}

		template <typename EntityType>
		eastl::shared_ptr<EntityType> SpawnNetworkEntity(OGameWorld* inOwningGameWorld, const eastl::string& inWorldLayer)
		{
			static_assert(eastl::is_base_of<AEntity, EntityType>::value, "Spawned EntityType must be of type AEntity or more derived");
			const TTypeInfo* entityTypeInfo = EntityType::StaticClass();
			return eastl::static_pointer_cast<EntityType>(SpawnNetworkEntity_Internal(*entityTypeInfo, inOwningGameWorld, inWorldLayer));
		}

		template <typename EntityType>
		eastl::shared_ptr<EntityType> SpawnNetworkObject(const TTypeInfo& inTypeInfo, OGameWorld* inOwningGameWorld, const eastl::string& inWorldLayer)
		{
			static_assert(eastl::is_base_of<AEntity, EntityType>::value, "Spawned EntityType must be of type AEntity or more derived");
			MAD_ASSERT_DESC(IsA<EntityType>(inTypeInfo), "Given type info must be a derived class of AEntity");
			return eastl::static_pointer_cast<ObjectType>(SpawnNetworkEntity_Internal(inTypeInfo, inOwningGameWorld, inWorldLayer));
		}

		void DestroyNetworkObject(eastl::shared_ptr<UObject> inObject);

	private:
		class UNetworkManager& m_networkManager;

		eastl::unique_ptr<UNetworkTransport> m_serverTransport;

		eastl::hash_map<NetworkPlayerID, eastl::shared_ptr<ONetworkPlayer>> m_players;

		SNetworkID::HandleType m_nextNetworkID;

		struct UNetObject
		{
			eastl::shared_ptr<UObject> Object;
			eastl::shared_ptr<UNetworkState> State;
			eastl::hash_map<NetworkPlayerID, eastl::shared_ptr<UNetworkObjectView>> NetworkViews;
		};
		eastl::hash_map<SNetworkID, UNetObject> m_netObjects;

		void SendNetworkStateUpdates();
		void UpdateNetworkStates();

		void ReceiveMessages();
		void ReceiveMessagesForPlayer(NetworkPlayerID inPlayerID);

		void AddNewNetworkPlayer(NetworkPlayerID inNewPlayerID);
		void RemoveNetworkPlayer(NetworkPlayerID inPlayerID);

		void SetPlayerID(eastl::shared_ptr<ONetworkPlayer> inPlayer, NetworkPlayerID inPlayerID);

		eastl::shared_ptr<UObject> SpawnNetworkObject_Internal(const TTypeInfo& inTypeInfo);
		eastl::shared_ptr<UObject> SpawnNetworkEntity_Internal(const TTypeInfo& inTypeInfo, OGameWorld* inOwningGameWorld, const eastl::string& inWorldLayer);
		void NetworkSpawn(eastl::shared_ptr<UObject> inObject, const TTypeInfo& inTypeInfo);

	protected:
		virtual void OnStart(int maxClients) override;
		virtual void OnStop() override;
		virtual void OnConnectionRequest(yojimbo::ServerConnectionRequestAction action, const yojimbo::ConnectionRequestPacket& packet, const yojimbo::Address& address, const yojimbo::ConnectToken& connectToken) override;
		virtual void OnChallengeResponse(yojimbo::ServerChallengeResponseAction action, const yojimbo::ChallengeResponsePacket& packet, const yojimbo::Address& address, const yojimbo::ChallengeToken& challengeToken) override;
		virtual void OnClientConnect(int clientIndex) override;
		virtual void OnClientDisconnect(int clientIndex) override;
		virtual void OnClientError(int clientIndex, yojimbo::ServerClientError error) override;
		virtual void OnPacketSent(int packetType, const yojimbo::Address& to, bool immediate) override;
		virtual void OnPacketReceived(int packetType, const yojimbo::Address& from) override;
		//virtual void OnConnectionPacketSent(yojimbo::Connection* connection, uint16_t sequence) override;
		//virtual void OnConnectionPacketAcked(yojimbo::Connection* connection, uint16_t sequence) override;
		//virtual void OnConnectionPacketReceived(yojimbo::Connection* connection, uint16_t sequence) override;
		//virtual void OnConnectionFragmentReceived(yojimbo::Connection* connection, uint16_t messageId, uint16_t fragmentId, int fragmentBytes, int channelId) override;
		//virtual bool ProcessUserPacket(int clientIndex, yojimbo::Packet* packet) override;

	protected:
		YOJIMBO_SERVER_MESSAGE_FACTORY(UGameMessageFactory);
	};
}
