#pragma once

#include <EASTL/unique_ptr.h>
#include <yojimbo/yojimbo.h>

#include "Networking/Network.h"
#include "Networking/NetworkClient.h"
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

	private:
		ENetMode m_netMode;
		yojimbo::ClientServerConfig m_config;

		eastl::unique_ptr<class UNetworkTransport> m_serverTransport;
		eastl::unique_ptr<class UNetworkTransport> m_clientTransport;

		eastl::unique_ptr<class UNetworkServer> m_server;
		eastl::unique_ptr<class UNetworkClient> m_client;

		bool StartServer(int port);
		bool ConnectToServer(const yojimbo::Address& inServerAddress);
	};
}
