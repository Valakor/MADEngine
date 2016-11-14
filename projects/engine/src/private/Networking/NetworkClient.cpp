#include "Networking/NetworkClient.h"

#include "Core/GameEngine.h"
#include "Misc/Logging.h"

using namespace yojimbo;

namespace MAD
{
	DECLARE_LOG_CATEGORY(LogNetworkClient);

	void UNetworkClient::OnConnect(const Address& address)
	{
		char addressString[MaxAddressLength];
		address.ToString(addressString, sizeof(addressString));
		LOG(LogNetworkClient, Log, "[OnConnect] Client connecting to %s...\n", addressString);
	}

	void UNetworkClient::OnClientStateChange(int previousState, int currentState)
	{
		(void)previousState;
		(void)currentState;

		MAD_ASSERT_DESC(previousState != currentState, "This should only be called when a client's state actually changes");

		LOG(LogNetworkClient, Log, "[OnClientStateChange] Client changed state from '%s' to '%s'\n", GetClientStateName(previousState), GetClientStateName(currentState));

		UNetworkManager& netManager = gEngine->GetNetworkManager();

		if (currentState == CLIENT_STATE_CONNECTED)
		{
			LOG(LogNetworkClient, Log, "[OnClientStateChange] Client connected as client %d\n", GetClientIndex());
			netManager.OnLocalPlayerConnected(GetClientIndex());
		}
		else if (currentState <= CLIENT_STATE_DISCONNECTED)
		{
			LOG(LogNetworkClient, Log, "[OnClientStateChange] Client %d disconnected\n", GetClientIndex());
			netManager.OnLocalPlayerDisconnectd();
		}
	}

	void UNetworkClient::OnDisconnect()
	{
		LOG(LogNetworkClient, Log, "[OnDisconnect] Client disconnecting...\n");
	}

	void UNetworkClient::OnPacketSent(int packetType, const Address& to, bool immediate)
	{
		(void)immediate;

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

#if NETWORK_VERBOSE_LOGGING
		char addressString[MaxAddressLength];
		to.ToString(addressString, sizeof(addressString));
		LOG(LogNetworkClient, Log, "[OnPacketSent] Client sent `%s` packet to %s%s\n", packetTypeString, addressString, immediate ? " (immediate)" : "");
#endif
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

#if NETWORK_VERBOSE_LOGGING
		char addressString[MaxAddressLength];
		from.ToString(addressString, sizeof(addressString));
		LOG(LogNetworkClient, Log, "[OnPacketReceived] Client received `%s` packet from %s\n", packetTypeString, addressString);
#endif
	}
}