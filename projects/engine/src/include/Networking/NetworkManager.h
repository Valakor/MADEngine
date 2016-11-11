#pragma once

#include <EASTL/hash_map.h>
#include <EASTL/shared_ptr.h>
#include <EASTL/vector.h>
#include <yojimbo/yojimbo.h>

#include "Networking/Network.h"
#include "Networking/NetworkClient.h"
#include "Networking/NetworkPlayer.h"
#include "Networking/NetworkServer.h"
#include "Networking/NetworkTransport.h"

namespace MAD
{
	class UNetworkManager
	{
	public:
		UNetworkManager();

		bool Init();
		void Tick(float inDeltaTime);
		void Shutdown();

		eastl::weak_ptr<ONetworkPlayer> GetLocalPlayer() const { return m_localPlayer; }
		eastl::weak_ptr<ONetworkPlayer> GetPlayerById(NetworkPlayerID inID) const;

		void OnLocalPlayerConnected(NetworkPlayerID inNewID);
		void OnLocalPlayerDisconnectd();

		void OnRemotePlayerConnected(NetworkPlayerID inNewID);
		void OnRemotePlayerDisconnected(NetworkPlayerID inID);

	private:
		ENetMode m_netMode;
		yojimbo::ClientServerConfig m_config;

		eastl::weak_ptr<ONetworkPlayer> m_localPlayer;
		eastl::hash_map<NetworkPlayerID, eastl::shared_ptr<ONetworkPlayer>> m_players;

		eastl::unique_ptr<UNetworkTransport> m_serverTransport;
		eastl::unique_ptr<UNetworkTransport> m_clientTransport;

		eastl::unique_ptr<UNetworkServer> m_server;
		eastl::unique_ptr<UNetworkClient> m_client;

		bool StartServer(int port);
		bool ConnectToServer(const yojimbo::Address& inServerAddress);

		void ClientReceiveMessages();

		void SetPlayerID(eastl::shared_ptr<ONetworkPlayer> inPlayer, NetworkPlayerID inPlayerID, bool inIsLocalPlayer);
		void AddConnectedPlayer(NetworkPlayerID inNewPlayerID);

	public:
		auto PlayersBegin() const -> decltype(m_players.cbegin()) { return m_players.cbegin(); };
		auto PlayersEnd() const -> decltype(m_players.cend()) { return m_players.cend(); };
	};
}
