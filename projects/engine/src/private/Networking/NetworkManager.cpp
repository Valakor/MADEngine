#include "Networking/NetworkManager.h"

#include "Core/GameEngine.h"
#include "Misc/Logging.h"
#include "Misc/Parse.h"

using namespace yojimbo;

namespace MAD
{
	DECLARE_LOG_CATEGORY(LogNetworkManager);

	UNetworkTransport::~UNetworkTransport()
	{
		ClearSendQueue();
		ClearReceiveQueue();
	}

	UNetworkManager::UNetworkManager()
		: m_isNetworkInitialized(false)
		, m_netMode(ENetMode::Client)
	{ }

	bool UNetworkManager::Init()
	{
		if (SParse::Find(SCmdLine::Get(), "-ListenServer"))
		{
			m_netMode = ENetMode::ListenServer;
		}

		if (SParse::Find(SCmdLine::Get(), "-DedicatedServer"))
		{
			m_netMode = ENetMode::DedicatedServer;
		}

		int port = SERVER_DEFAULT_LISTEN_PORT;
		SParse::Get(SCmdLine::Get(), "-Port=", port);

		if (m_netMode != ENetMode::Client)
		{
			if (!InitializeYojimbo())
			{
				LOG(LogNetworkManager, Error, "Failed to initialize Yojimbo\n");
				return false;
			}

			if (!StartServer(port))
			{
				LOG(LogNetworkManager, Error, "Failed to start server on port %i\n", port);
				return false;
			}

			m_isNetworkInitialized = true;
		}

		return true;
	}

	void UNetworkManager::Tick(float inDeltaTime)
	{
		(void)inDeltaTime;

		/*m_server->SendPackets();
		m_transport->WritePackets();
		m_transport->ReadPackets();
		m_server->ReceivePackets();
		m_server->CheckForTimeOut();
		m_server->AdvanceTime(inDeltaTime);*/
	}

	void UNetworkManager::Shutdown()
	{
		if (m_server)
		{
			m_server->Stop();
		}

		if (m_transport)
		{
			m_transport->Reset();
		}

		m_server = nullptr;
		m_transport = nullptr;

		ShutdownYojimbo();
	}

	bool UNetworkManager::StartServer(int port)
	{
		Address bindAddr("0.0.0.0", port);
		Address publicAddr("127.0.0.1", port);

		m_transport = eastl::make_unique<UNetworkTransport>(bindAddr);
		if (m_transport->GetError() != SOCKET_ERROR_NONE)
		{
			return false;
		}

		m_server = eastl::make_unique<UNetworkServer>(GetDefaultAllocator(), *(m_transport.get()));
		m_server->SetServerAddress(publicAddr);
		m_server->SetFlags(SERVER_FLAG_ALLOW_INSECURE_CONNECT);
		m_transport->SetFlags(TRANSPORT_FLAG_INSECURE_MODE);
		m_server->Start();

		return true;
	}
}
