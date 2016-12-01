#include "Networking/NetworkManager.h"

#include "Core/GameEngine.h"
#include "Misc/ErrorHandling.h"
#include "Misc/Logging.h"
#include "Misc/Parse.h"

using namespace yojimbo;

namespace MAD
{
	DECLARE_LOG_CATEGORY(LogNetworkManager);

	UNetworkManager::UNetworkManager()
		: m_netMode(ENetMode::Client)
	{
		m_config.connectionConfig.numChannels = NUM_CHANNELS;
		m_config.connectionConfig.channelConfig[UNRELIABLE_CHANNEL].type = CHANNEL_TYPE_UNRELIABLE_UNORDERED;
		m_config.connectionConfig.channelConfig[RELIABLE_CHANNEL].type = CHANNEL_TYPE_RELIABLE_ORDERED;
	}

	bool UNetworkManager::Init()
	{
		LOG(LogNetworkManager, Log, "Network manager initialization begin...\n");

		if (!InitializeYojimbo())
		{
			LOG(LogNetworkManager, Error, "Failed to initialize Yojimbo\n");
			return false;
		}

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

		if (m_netMode == ENetMode::DedicatedServer || m_netMode == ENetMode::ListenServer)
		{
			if (!StartServer(port))
			{
				LOG(LogNetworkManager, Error, "Failed to start server on port %i\n", port);
				return false;
			}
		}

		if (m_netMode == ENetMode::Client || m_netMode == ENetMode::ListenServer)
		{
			// Connect to server if specified on commandline
			eastl::string connectString = "127.0.0.1";
			if (m_netMode == ENetMode::ListenServer || SParse::Get(SCmdLine::Get(), "-ServerEndpoint=", connectString))
			{
				Address connectAddr(connectString.c_str(), port);
				if (!connectAddr.IsValid())
				{
					LOG(LogNetworkManager, Error, "Server endpoint is invalid\n");
					return false;
				}

				if (!ConnectToServer(connectAddr))
				{
					LOG(LogNetworkManager, Error, "Failed to initiate client connection to server at %s:%i\n", connectString.c_str(), port);
					return false;
				}
			}
		}

		LOG(LogNetworkManager, Log, "Network manager initialization successful\n");
		return true;
	}

	void UNetworkManager::Tick(float inDeltaTime)
	{
		(void)inDeltaTime;
		float gameTime = gEngine->GetGameTime();

		if (m_server)
		{
			m_server->Tick(gameTime);
		}

		if (m_client)
		{
			m_client->Tick(gameTime);
		}
	}

	void UNetworkManager::Shutdown()
	{
		if (m_server)
		{
			m_server->Stop();
		}

		if (m_client)
		{
			m_client->Disconnect();
		}

		m_server = nullptr;
		m_client = nullptr;

		ShutdownYojimbo();
	}

	bool UNetworkManager::StartServer(int port)
	{
		float gameTime = gEngine->GetGameTime();

		Address bindAddr("0.0.0.0", port);
		Address publicAddr("127.0.0.1", port);

		eastl::unique_ptr<UNetworkTransport> serverTransport = eastl::make_unique<UNetworkTransport>(gameTime, bindAddr);
		if (serverTransport->GetError() != SOCKET_ERROR_NONE)
		{
			return false;
		}

		serverTransport->SetFlags(TRANSPORT_FLAG_INSECURE_MODE);
		m_server = eastl::make_unique<UNetworkServer>(*this, eastl::move(serverTransport), gameTime, m_config);
		m_server->SetServerAddress(publicAddr);
		m_server->SetFlags(SERVER_FLAG_ALLOW_INSECURE_CONNECT);
		m_server->Start();

		return true;
	}

	bool UNetworkManager::ConnectToServer(const Address& inServerAddress)
	{
		float gameTime = gEngine->GetGameTime();

		eastl::unique_ptr<UNetworkTransport> clientTransport = eastl::make_unique<UNetworkTransport>(gameTime);
		if (clientTransport->GetError() != SOCKET_ERROR_NONE)
		{
			return false;
		}

		clientTransport->SetFlags(TRANSPORT_FLAG_INSECURE_MODE);

		uint64_t clientID = 0;
		yojimbo::RandomBytes((uint8_t*)&clientID, 8); // TODO: Is there a better way to do this?

		m_client = eastl::make_unique<UNetworkClient>(*this, eastl::move(clientTransport), gameTime, m_config);
		m_client->InsecureConnect(clientID, inServerAddress);

		return true;
	}

	size_t UNetworkManager::GetNumPlayers() const
	{
		switch (m_netMode)
		{
		case ENetMode::DedicatedServer:
			return m_server->GetNumConnectedPlayers();

		case ENetMode::ListenServer:
		case ENetMode::Client:
			return m_client->GetNumPlayers();

		default:
			UNREACHABLE("Unrecognized Net Mode");
			break;
		}
	}

	void UNetworkManager::DestroyNetworkObject(UObject& inObject)
	{
		if (!m_server) return;

		m_server->DestroyNetworkObject(inObject);
	}

	void UNetworkManager::SendNetworkEvent(EEventTarget inEventTarget, EEventTypes inEventType, UObject& inTargetObject, void* inEventData, size_t inEventSize, NetworkPlayerID inTargetPlayer /*= InvalidPlayerID*/)
	{
		if (!inEventData && inEventSize > 0)
		{
			LOG(LogNetworkManager, Warning, "Trying to send event with invalid data\n");
			return;
		}

		if (!inTargetObject.IsNetworkSpawned())
		{
			LOG(LogNetworkManager, Warning, "Trying to send event to non-networked object!\n");
			return;
		}

		if (m_netMode == ENetMode::Client || (m_netMode == ENetMode::ListenServer && inEventTarget == EEventTarget::Server))
		{
			if (inEventTarget != EEventTarget::Server)
			{
				LOG(LogNetworkManager, Warning, "Attempting to send non-server event from client\n");
				return;
			}

			m_client->SendNetworkEvent(inEventType, inTargetObject, inEventData, inEventSize);

			return;
		}
		else if (m_netMode == ENetMode::DedicatedServer || (m_netMode == ENetMode::ListenServer && inEventTarget != EEventTarget::Server))
		{
			if (inEventTarget == EEventTarget::Server)
			{
				LOG(LogNetworkManager, Warning, "Attempting to send non-client event from server\n");
				return;
			}

			if (inEventTarget == EEventTarget::Client && m_server->GetPlayerByID(inTargetPlayer).expired())
			{
				LOG(LogNetworkManager, Warning, "Attempting to send client event with invalid player ID\n");
				return;
			}

			m_server->SendNetworkEvent(inEventTarget, inEventType, inTargetObject, inEventData, inEventSize, inTargetPlayer);

			return;
		}
		
		UNREACHABLE("Our logic is wrong if you got here!!!");
	}
}
