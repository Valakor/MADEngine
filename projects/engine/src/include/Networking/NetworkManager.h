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

		ENetMode GetNetMode() const { return m_netMode; }

		eastl::weak_ptr<ONetworkPlayer> GetLocalPlayer() const { return m_client ? m_client->GetLocalPlayer() : eastl::weak_ptr<ONetworkPlayer>(); }
		size_t GetNumPlayers() const;

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
