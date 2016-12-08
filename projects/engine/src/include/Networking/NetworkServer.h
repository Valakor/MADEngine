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

		void PreTick(double inGameTime);
		void PostTick();

		size_t GetNumConnectedPlayers() const { return m_players.size(); }
		eastl::weak_ptr<ONetworkPlayer> GetPlayerByID(NetworkPlayerID inID) const;

		eastl::shared_ptr<UObject> GetNetworkObject(SNetworkID inNetworkID) const;

		void DestroyNetworkObject(UObject& inObject);

		void SendNetworkEvent(EEventTarget inEventTarget, EEventTypes inEventType, UObject& inTargetObject, void* inEventData, size_t inEventSize, NetworkPlayerID inTargetPlayer);
	
		eastl::shared_ptr<UObject> SpawnNetworkObject(const TTypeInfo& inTypeInfo, ONetworkPlayer& inNetOwner);
		eastl::shared_ptr<UObject> SpawnNetworkEntity(const TTypeInfo& inTypeInfo, ONetworkPlayer& inNetOwner, OGameWorld* inOwningGameWorld, const eastl::string& inWorldLayer);
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

		eastl::vector<eastl::pair<UNetObject, TypeID>> m_deferredSpawnMessages;

		void SendNetworkStateUpdates();

		void ReceiveMessages();
		void ReceiveMessagesForPlayer(const ONetworkPlayer& inPlayer);
		void ReceiveMessagesForPlayer(const ONetworkPlayer& inPlayer, int inChannelID);

		void HandleEventMessage(MEvent& message, const ONetworkPlayer& inPlayer);

		eastl::shared_ptr<ONetworkPlayer> AddNewNetworkPlayer(NetworkPlayerID inNewPlayerID);
		void RemoveNetworkPlayer(NetworkPlayerID inPlayerID);

		void SetPlayerID(eastl::shared_ptr<ONetworkPlayer> inPlayer, NetworkPlayerID inPlayerID);

		// Doesn't spawn on the network immediately, defers until PostTick
		void NetworkSpawn(eastl::shared_ptr<UObject> inObject, const TTypeInfo& inTypeInfo);
		void FlushNetworkSpawns();

		void SendNetObjectsToNewPlayer(const ONetworkPlayer& inPlayer);
		void DestroyNetObjectsForPlayer(const ONetworkPlayer& inPlayer);

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
