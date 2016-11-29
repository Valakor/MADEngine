#pragma once

#include <EASTL/hash_map.h>
#include <EASTL/weak_ptr.h>
#include <EASTL/unique_ptr.h>
#include <yojimbo/yojimbo.h>

#include "Networking/NetworkTypes.h"
#include "Networking/NetworkTransport.h"
#include "Networking/NetworkPlayer.h"

namespace MAD
{
	class UNetworkClient : public yojimbo::Client
	{
	public:
		explicit UNetworkClient(class UNetworkManager& inNetworkManager, eastl::unique_ptr<UNetworkTransport> inClientTransport, double inCurrentGameTime, const yojimbo::ClientServerConfig& inClientConfig = yojimbo::ClientServerConfig());

		void Tick(float inGameTime);

		eastl::weak_ptr<ONetworkPlayer> GetLocalPlayer() const { return m_localPlayer; }

		size_t GetNumPlayers() const { return m_players.size(); }
		eastl::weak_ptr<ONetworkPlayer> GetPlayerByID(NetworkPlayerID inID) const;

	private:
		class UNetworkManager& m_networkManager;

		eastl::unique_ptr<UNetworkTransport> m_clientTransport;

		eastl::weak_ptr<ONetworkPlayer> m_localPlayer;
		eastl::hash_map<NetworkPlayerID, eastl::shared_ptr<ONetworkPlayer>> m_players;

		struct UNetObject
		{
			eastl::shared_ptr<UObject> Object;
			eastl::shared_ptr<UNetworkState> State;
			eastl::shared_ptr<UNetworkObjectView> NetworkView;
		};
		eastl::hash_map<SNetworkID, UNetObject> m_netObjects;

		void ReceiveMessages();
		void HandleCreateObjectMessage(MCreateObject& message);
		void HandleUpdateObjectMessage(MUpdateObject& message);
		void HandleDestroyObjectMessage(MDestroyObject& message);

		void SetPlayerID(eastl::shared_ptr<ONetworkPlayer> inPlayer, NetworkPlayerID inPlayerID, bool inIsLocalPlayer);

		void OnConnected();
		void OnDisconnected();

		void OnRemotePlayerConnected(NetworkPlayerID inNewPlayerID);
		void OnRemotePlayerDisconnected(NetworkPlayerID inPlayerID);

	protected:
		virtual void OnConnect(const yojimbo::Address& address) override;
		virtual void OnClientStateChange(int previousState, int currentState) override;
		virtual void OnDisconnect() override;
		virtual void OnPacketSent(int packetType, const yojimbo::Address& to, bool immediate) override;
		virtual void OnPacketReceived(int packetType, const yojimbo::Address& from) override;
		//virtual void OnConnectionPacketSent(yojimbo::Connection* connection, uint16_t sequence) override;
		//virtual void OnConnectionPacketAcked(yojimbo::Connection* connection, uint16_t sequence) override;
		//virtual void OnConnectionPacketReceived(yojimbo::Connection* connection, uint16_t sequence) override;
		//virtual void OnConnectionFragmentReceived(yojimbo::Connection* connection, uint16_t messageId, uint16_t fragmentId, int fragmentBytes, int channelId) override;
		//virtual bool ProcessUserPacket(yojimbo::Packet* packet) override;

	protected:
		YOJIMBO_CLIENT_MESSAGE_FACTORY(UGameMessageFactory);
	};
}
