#include "Networking/NetworkManager.h"

#include "Core/GameEngine.h"
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

		if (m_netMode == ENetMode::Client)
		{
			// Create local player
			eastl::shared_ptr<ONetworkPlayer> localPlayer = CreateDefaultObject<ONetworkPlayer>(nullptr);
			SetPlayerID(localPlayer, InvalidPlayerID, true);

			// Connect to server if specified on commandline
			eastl::string connectString;
			if (SParse::Get(SCmdLine::Get(), "-ServerEndpoint=", connectString))
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
		else
		{
			if (!StartServer(port))
			{
				LOG(LogNetworkManager, Error, "Failed to start server on port %i\n", port);
				return false;
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
			m_server->AdvanceTime(gameTime);
			m_serverTransport->AdvanceTime(gameTime);

			m_serverTransport->ReadPackets();
			m_server->ReceivePackets();
			m_server->CheckForTimeOut();

			// Insert logic to create message for RPCs, state updates, etc.
			m_server->SendPackets();
			m_serverTransport->WritePackets();
		}
		else if (m_client)
		{
			m_client->AdvanceTime(gameTime);
			m_clientTransport->AdvanceTime(gameTime);

			m_clientTransport->ReadPackets();
			m_client->ReceivePackets();

			m_client->CheckForTimeOut();
			
			if (m_client->IsConnected())
			{
				ClientReceiveMessages();
			}

			m_client->SendPackets();
			m_clientTransport->WritePackets();
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

		if (m_serverTransport)
		{
			m_serverTransport->Reset();
		}

		if (m_clientTransport)
		{
			m_clientTransport->Reset();
		}

		m_server = nullptr;
		m_client = nullptr;
		m_serverTransport = nullptr;
		m_clientTransport = nullptr;

		ShutdownYojimbo();
	}

	void UNetworkManager::OnLocalPlayerConnected(NetworkPlayerID inNewID)
	{
		auto localPlayer = m_localPlayer.lock();
		SetPlayerID(localPlayer, inNewID, true);
	}

	void UNetworkManager::OnLocalPlayerDisconnectd()
	{
		auto localPlayer = m_localPlayer.lock();
		m_players.clear();
		SetPlayerID(localPlayer, InvalidPlayerID, true);
	}

	void UNetworkManager::OnRemotePlayerConnected(NetworkPlayerID inNewID)
	{
		AddConnectedPlayer(inNewID);
	}

	void UNetworkManager::OnRemotePlayerDisconnected(NetworkPlayerID inID)
	{
		MAD_ASSERT_DESC(m_localPlayer.expired() || m_localPlayer.lock()->GetPlayerID() != inID, "The remote disconnected player cannot be this local player");

		auto erased = m_players.erase(inID);
		if (!erased)
		{
			LOG(LogNetworkManager, Warning, "[OnRemotePlayerDisconnected] Remote player with ID %d not found\n", inID);
		}
		else
		{
			LOG(LogNetworkManager, Log, "[OnRemotePlayerDisconnected] Remote player %d disconnected\n", inID);
		}
	}

	bool UNetworkManager::StartServer(int port)
	{
		Address bindAddr("0.0.0.0", port);
		Address publicAddr("127.0.0.1", port);

		m_serverTransport = eastl::make_unique<UNetworkTransport>(bindAddr);
		if (m_serverTransport->GetError() != SOCKET_ERROR_NONE)
		{
			return false;
		}

		m_server = eastl::make_unique<UNetworkServer>(GetDefaultAllocator(), *(m_serverTransport.get()), m_config);
		m_server->SetServerAddress(publicAddr);
		m_server->SetFlags(SERVER_FLAG_ALLOW_INSECURE_CONNECT);
		m_serverTransport->SetFlags(TRANSPORT_FLAG_INSECURE_MODE);
		m_server->Start();

		return true;
	}

	bool UNetworkManager::ConnectToServer(const Address& inServerAddress)
	{
		m_clientTransport = eastl::make_unique<UNetworkTransport>();
		if (m_clientTransport->GetError() != SOCKET_ERROR_NONE)
		{
			return false;
		}

		m_clientTransport->SetFlags(TRANSPORT_FLAG_INSECURE_MODE);

		m_client = eastl::make_unique<UNetworkClient>(GetDefaultAllocator(), *(m_clientTransport.get()), m_config);
		m_client->InsecureConnect(inServerAddress);

		return true;
	}

	void UNetworkManager::ClientReceiveMessages()
	{
		MAD_ASSERT_DESC(m_client && m_client->IsConnected(), "Need a connected client to call this");

		while (auto msg = m_client->ReceiveMsg())
		{
			switch (msg->GetType())
			{
			case OTHER_PLAYER_CONNECTION_CHANGED:
			{
				MOtherPlayerConnectionChanged* message = static_cast<MOtherPlayerConnectionChanged*>(msg);
				if (message->m_connect)
				{
					OnRemotePlayerConnected(message->m_playerID);
				}
				else
				{
					OnRemotePlayerDisconnected(message->m_playerID);
				}
				break;
			}
			}

			m_client->ReleaseMsg(msg);
		}
	}

	eastl::weak_ptr<ONetworkPlayer> UNetworkManager::GetPlayerById(NetworkPlayerID inID) const
	{
		auto iter = m_players.find(inID);
		return (iter != m_players.end()) ? iter->second : nullptr;
	}

	void UNetworkManager::SetPlayerID(eastl::shared_ptr<ONetworkPlayer> inPlayer, NetworkPlayerID inPlayerID, bool inIsLocalPlayer)
	{
		MAD_ASSERT_DESC(m_players.find(inPlayerID) == m_players.end(), "Cannot assign a player to an already mapped playerID");

		auto iter = m_players.find(inPlayer->GetPlayerID());
		if (iter != m_players.end())
		{
			m_players.erase(iter);
		}

		inPlayer->SetPlayerID(inPlayerID);
		inPlayer->SetIsLocalPlayer(inIsLocalPlayer);

		m_players.insert({ inPlayerID, inPlayer });

		if (inIsLocalPlayer)
		{
			m_localPlayer = inPlayer;
		}
	}

	void UNetworkManager::AddConnectedPlayer(NetworkPlayerID inNewPlayerID)
	{
		auto newPlayer = CreateDefaultObject<ONetworkPlayer>(nullptr);
		SetPlayerID(newPlayer, inNewPlayerID, false);
		LOG(LogNetworkManager, Log, "[AddConnectedPlayer] Remote player connected with id %d\n", inNewPlayerID);
	}
}
