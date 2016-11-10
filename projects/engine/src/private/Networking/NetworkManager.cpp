#include "Networking/NetworkManager.h"

#include <time.h>

#include "Core/GameEngine.h"
#include "Misc/Logging.h"
#include "Misc/Parse.h"

using namespace yojimbo;

namespace MAD
{
	DECLARE_LOG_CATEGORY(LogNetworkTransport);
	DECLARE_LOG_CATEGORY(LogNetworkServer);
	DECLARE_LOG_CATEGORY(LogNetworkClient);
	DECLARE_LOG_CATEGORY(LogNetworkManager);

#define verbose_logging 0

	//---------------------------------------//
	//           Network Transport           //
	//---------------------------------------//

	UNetworkTransport::~UNetworkTransport()
	{
		ClearSendQueue();
		ClearReceiveQueue();
	}

	//---------------------------------------//
	//             Network Server            //
	//---------------------------------------//

	void UNetworkServer::OnStart(int maxClients)
	{
		LOG(LogNetworkServer, Log, "[OnStart] MaxClients=%i\n", maxClients);
	}

	void UNetworkServer::OnStop()
	{
		LOG(LogNetworkServer, Log, "[OnStop]\n");
	}

	void UNetworkServer::OnConnectionRequest(ServerConnectionRequestAction action, const ConnectionRequestPacket& packet, const Address& address, const ConnectToken& connectToken)
	{
		char addressString[MaxAddressLength];
		address.ToString(addressString, sizeof(addressString));

		switch (action)
		{
		case SERVER_CONNECTION_REQUEST_DENIED_SERVER_IS_FULL:
			LOG(LogNetworkServer, Warning, "[OnConnectionRequest] Denied connection request from %s. Server is full\n", addressString);
			break;

		case SERVER_CONNECTION_REQUEST_IGNORED_BECAUSE_FLAG_IS_SET:
			LOG(LogNetworkServer, Warning, "[OnConnectionRequest] Ignored connection request from %s. Flag is set to ignore connection requests\n", addressString);
			break;

		case SERVER_CONNECTION_REQUEST_IGNORED_CONNECT_TOKEN_EXPIRED:
			LOG(LogNetworkServer, Warning, "[OnConnectionRequest] Ignored connection request from %s. Connect token has expired (expire timestamp = %" PRIx64 ", current time = %" PRIx64 ")\n", addressString, packet.connectTokenExpireTimestamp, (uint64_t) ::time(NULL));
			break;

		case SERVER_CONNECTION_REQUEST_IGNORED_FAILED_TO_DECRYPT_CONNECT_TOKEN:
			LOG(LogNetworkServer, Warning, "[OnConnectionRequest] Ignored connection request from %s. Failed to decrypt connect token\n", addressString);
			break;

		case SERVER_CONNECTION_REQUEST_IGNORED_SERVER_ADDRESS_NOT_IN_WHITELIST:
			LOG(LogNetworkServer, Warning, "[OnConnectionRequest] Ignored connection request from %s. Server address not in whitelist\n", addressString);
			break;

		case SERVER_CONNECTION_REQUEST_IGNORED_CLIENT_ID_IS_ZERO:
			LOG(LogNetworkServer, Warning, "[OnConnectionRequest] Ignored connection request from %s. Client ID is zero\n", addressString);
			break;

		case SERVER_CONNECTION_REQUEST_IGNORED_ADDRESS_ALREADY_CONNECTED:
			LOG(LogNetworkServer, Warning, "[OnConnectionRequest] Ignored connection request from %s. Address already connected\n", addressString);
			break;

		case SERVER_CONNECTION_REQUEST_IGNORED_CLIENT_ID_ALREADY_CONNECTED:
			LOG(LogNetworkServer, Warning, "[OnConnectionRequest] Ignored connection request from %s. Client id %" PRIx64 " already connected\n", addressString, connectToken.clientId);
			break;

		case SERVER_CONNECTION_REQUEST_IGNORED_FAILED_TO_ADD_ENCRYPTION_MAPPING:
			LOG(LogNetworkServer, Warning, "[OnConnectionRequest] Ignored connection request from %s. Failed to add encryption mapping\n", addressString);
			break;

		case SERVER_CONNECTION_REQUEST_IGNORED_CONNECT_TOKEN_ALREADY_USED:
			LOG(LogNetworkServer, Warning, "[OnConnectionRequest] Ignored connection request from %s. Connect token already used\n", addressString);
			break;

		case SERVER_CONNECTION_REQUEST_IGNORED_FAILED_TO_GENERATE_CHALLENGE_TOKEN:
			LOG(LogNetworkServer, Warning, "[OnConnectionRequest] Ignored connection request from %s. Failed to generate challenge token\n", addressString);
			break;

		case SERVER_CONNECTION_REQUEST_IGNORED_FAILED_TO_ALLOCATE_CHALLENGE_PACKET:
			LOG(LogNetworkServer, Warning, "[OnConnectionRequest] Ignored connection request from %s. Failed to allocate challenge packet\n", addressString);
			break;

		case SERVER_CONNECTION_REQUEST_IGNORED_FAILED_TO_ENCRYPT_CHALLENGE_TOKEN:
			LOG(LogNetworkServer, Warning, "[OnConnectionRequest] Ignored connection request from %s. Failed to encrypt challenge token\n", addressString);
			break;

		default:
			break;
		}
	}

	void UNetworkServer::OnChallengeResponse(ServerChallengeResponseAction action, const ChallengeResponsePacket& packet, const Address& address, const ChallengeToken& challengeToken)
	{
		(void)packet;

		char addressString[MaxAddressLength];
		address.ToString(addressString, sizeof(addressString));

		switch (action)
		{
		case SERVER_CHALLENGE_RESPONSE_ACCEPTED:
			LOG(LogNetworkServer, Log, "[OnChallengeResponse] Accepted challenge response from %s\n", addressString);
			break;

		case SERVER_CHALLENGE_RESPONSE_DENIED_SERVER_IS_FULL:
			LOG(LogNetworkServer, Warning, "[OnChallengeResponse] Denied challenge response from %s. Server is full\n", addressString);
			break;

		case SERVER_CHALLENGE_RESPONSE_IGNORED_BECAUSE_FLAG_IS_SET:
			LOG(LogNetworkServer, Warning, "[OnChallengeResponse] Ignored challenge response from %s. Flag is set to ignore challenge responses\n", addressString);
			break;

		case SERVER_CHALLENGE_RESPONSE_IGNORED_FAILED_TO_DECRYPT_CHALLENGE_TOKEN:
			LOG(LogNetworkServer, Warning, "[OnChallengeResponse] Ignored challenge response from %s. Failed to decrypt challenge token\n", addressString);
			break;

		case SERVER_CHALLENGE_RESPONSE_IGNORED_ADDRESS_ALREADY_CONNECTED:
			LOG(LogNetworkServer, Warning, "[OnChallengeResponse] Ignored challenge response from %s. Address already connected\n", addressString, challengeToken.clientId);
			break;

		case SERVER_CHALLENGE_RESPONSE_IGNORED_CLIENT_ID_ALREADY_CONNECTED:
			LOG(LogNetworkServer, Warning, "[OnChallengeResponse] Ignored challenge response from %s. Client id %" PRIx64 " already connected\n", addressString, challengeToken.clientId);
			break;

		default:
			break;
		}
	}

	void UNetworkServer::OnClientConnect(int clientIndex)
	{
		char addressString[MaxAddressLength];
		GetClientAddress(clientIndex).ToString(addressString, sizeof(addressString));
		LOG(LogNetworkServer, Log, "[OnClientConnect] Client %d connected (client address = %s, client id = %.16" PRIx64 ")\n", clientIndex, addressString, GetClientId(clientIndex));
	}

	void UNetworkServer::OnClientDisconnect(int clientIndex)
	{
		char addressString[MaxAddressLength];
		GetClientAddress(clientIndex).ToString(addressString, sizeof(addressString));
		LOG(LogNetworkServer, Log, "[OnClientDisconnect] Client %d disconnected (client address = %s, client id = %.16" PRIx64 ")\n", clientIndex, addressString, GetClientId(clientIndex));
	}

	void UNetworkServer::OnClientError(int clientIndex, ServerClientError error)
	{
		auto clientId = GetClientId(clientIndex);

		switch (error)
		{
		case SERVER_CLIENT_ERROR_TIMEOUT:
			LOG(LogNetworkServer, Warning, "[OnClientError] Client %d timed out (client id = %.16" PRIx64 ")\n", clientIndex, clientId);
			break;

		case SERVER_CLIENT_ERROR_ALLOCATOR:
			LOG(LogNetworkServer, Warning, "[OnClientError] Client %d experienced an allocator error (client id = %.16" PRIx64 ")\n", clientIndex, clientId);
			break;

		case SERVER_CLIENT_ERROR_CONNECTION:
			LOG(LogNetworkServer, Warning, "[OnClientError] Client %d experienced a connection error (client id = %.16" PRIx64 ")\n", clientIndex, clientId);
			break;

		case SERVER_CLIENT_ERROR_MESSAGE_FACTORY:
			LOG(LogNetworkServer, Warning, "[OnClientError] Client %d experienced a message factory error (client id = %.16" PRIx64 ")\n", clientIndex, clientId);
			break;

		case SERVER_CLIENT_ERROR_PACKET_FACTORY:
			LOG(LogNetworkServer, Warning, "[OnClientError] Client %d experienced a packet factory error (client id = %.16" PRIx64 ")\n", clientIndex, clientId);
			break;

		default:
			break;
		}
	}

	void UNetworkServer::OnPacketSent(int packetType, const Address& to, bool immediate)
	{
		const char* packetTypeString = nullptr;

		switch (packetType)
		{
		case CLIENT_SERVER_PACKET_CONNECTION_DENIED:          packetTypeString = "connection denied";     break;
		case CLIENT_SERVER_PACKET_CHALLENGE:                  packetTypeString = "challenge";             break;
		case CLIENT_SERVER_PACKET_KEEPALIVE:                  packetTypeString = "keep alive";            break;
		case CLIENT_SERVER_PACKET_DISCONNECT:                 packetTypeString = "disconnect";            break;

		default:
			return;
		}

		if (verbose_logging)
		{
			char addressString[MaxAddressLength];
			to.ToString(addressString, sizeof(addressString));
			LOG(LogNetworkServer, Log, "[OnPacketSent] Server sent `%s` packet to %s%s\n", packetTypeString, addressString, immediate ? " (immediate)" : "");
		}
	}

	void UNetworkServer::OnPacketReceived(int packetType, const Address& from)
	{
		const char* packetTypeString = nullptr;

		switch (packetType)
		{
		case CLIENT_SERVER_PACKET_CONNECTION_REQUEST:         packetTypeString = "connection request";        break;
		case CLIENT_SERVER_PACKET_CHALLENGE_RESPONSE:         packetTypeString = "challenge response";        break;
		case CLIENT_SERVER_PACKET_KEEPALIVE:                  packetTypeString = "keep alive";                break;
		case CLIENT_SERVER_PACKET_DISCONNECT:                 packetTypeString = "disconnect";                break;

		default:
			return;
		}

		if (verbose_logging)
		{
			char addressString[MaxAddressLength];
			from.ToString(addressString, sizeof(addressString));
			LOG(LogNetworkServer, Log, "[OnPacketReceived] Server received `%s` packet from %s\n", packetTypeString, addressString);
		}
	}

	bool UNetworkServer::ProcessUserPacket(int clientIndex, yojimbo::Packet* packet)
	{
		(void)clientIndex;

		if (packet->GetType() == PACKET_A)
		{
			return true;
		}

		return false;
	}

	//---------------------------------------//
	//             Network Client            //
	//---------------------------------------//

	void UNetworkClient::OnConnect(const Address& address)
	{
		char addressString[MaxAddressLength];
		address.ToString(addressString, sizeof(addressString));
		LOG(LogNetworkClient, Log, "[OnConnect] Client connecting to %s\n", addressString);
	}

	void UNetworkClient::OnClientStateChange(int previousState, int currentState)
	{
		assert(previousState != currentState);
		const char* previousStateString = GetClientStateName(previousState);
		const char* currentStateString = GetClientStateName(currentState);
		LOG(LogNetworkClient, Log, "[OnClientStateChange] Client changed state from '%s' to '%s'\n", previousStateString, currentStateString);

		if (currentState == CLIENT_STATE_CONNECTED)
		{
			LOG(LogNetworkClient, Log, "[OnClientStateChange] Client connected as client %d\n", GetClientIndex());
			printf("client connected as client %d\n", GetClientIndex());
		}
	}

	void UNetworkClient::OnDisconnect()
	{
		LOG(LogNetworkClient, Log, "[OnDisconnect] Client disconnected\n");
	}

	void UNetworkClient::OnPacketSent(int packetType, const Address& to, bool immediate)
	{
		const char* packetTypeString = nullptr;

		switch (packetType)
		{
		case CLIENT_SERVER_PACKET_CONNECTION_REQUEST:         packetTypeString = "connection request";        break;
		case CLIENT_SERVER_PACKET_CHALLENGE_RESPONSE:         packetTypeString = "challenge response";        break;
		case CLIENT_SERVER_PACKET_KEEPALIVE:                  packetTypeString = "keep alive";                break;
		case CLIENT_SERVER_PACKET_DISCONNECT:                 packetTypeString = "disconnect";                break;

		default:
			return;
		}

		if (verbose_logging)
		{
			char addressString[MaxAddressLength];
			to.ToString(addressString, sizeof(addressString));
			LOG(LogNetworkClient, Log, "[OnPacketSent] Client sent `%s` packet to %s%s\n", packetTypeString, addressString, immediate ? " (immediate)" : "");
		}
	}

	void UNetworkClient::OnPacketReceived(int packetType, const Address& from)
	{
		const char* packetTypeString = nullptr;

		switch (packetType)
		{
		case CLIENT_SERVER_PACKET_CONNECTION_DENIED:          packetTypeString = "connection denied";     break;
		case CLIENT_SERVER_PACKET_CHALLENGE:                  packetTypeString = "challenge";             break;
		case CLIENT_SERVER_PACKET_KEEPALIVE:                  packetTypeString = "keep alive";            break;
		case CLIENT_SERVER_PACKET_DISCONNECT:                 packetTypeString = "disconnect";            break;

		default:
			return;
		}

		if (verbose_logging)
		{
			char addressString[MaxAddressLength];
			from.ToString(addressString, sizeof(addressString));
			LOG(LogNetworkClient, Log, "[OnPacketReceived] Client received `%s` packet from %s\n", packetTypeString, addressString);
		}
	}

	bool UNetworkClient::ProcessUserPacket(Packet* packet)
	{
		if (packet->GetType() == PACKET_A)
		{
			return true;
		}

		return false;
	}

	//---------------------------------------//
	//            Network Manager            //
	//---------------------------------------//

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
			// Insert logic to create message for RPCs, state updates, etc.

			m_server->SendPackets();
			m_serverTransport->WritePackets();
			m_serverTransport->ReadPackets();
			m_server->ReceivePackets();
			m_server->CheckForTimeOut();
			m_server->AdvanceTime(gameTime);
			m_serverTransport->AdvanceTime(gameTime);
		}
		else if (m_client)
		{
			m_client->SendPackets();
			m_clientTransport->WritePackets();
			m_clientTransport->ReadPackets();
			m_client->ReceivePackets();
			m_client->CheckForTimeOut();
			
			if (m_client->IsDisconnected())
			{
				// ??
			}

			m_client->AdvanceTime(gameTime);
			m_clientTransport->AdvanceTime(gameTime);

			if (m_client->ConnectionFailed())
			{
				// ??
			}
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
}
