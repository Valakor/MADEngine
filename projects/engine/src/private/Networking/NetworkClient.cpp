#include "Networking/NetworkClient.h"

#include "Core/GameEngine.h"
#include "Misc/Logging.h"

using namespace yojimbo;

namespace MAD
{
	DECLARE_LOG_CATEGORY(LogNetworkClient);

	UNetworkClient::UNetworkClient(eastl::unique_ptr<UNetworkTransport> inClientTransport, double inCurrentGameTime, const ClientServerConfig& inClientConfig)
		: Client(GetDefaultAllocator(), *inClientTransport, inClientConfig, inCurrentGameTime)
		, m_clientTransport(eastl::move(inClientTransport))
	{

	}

	void UNetworkClient::Tick(float inGameTime)
	{
		AdvanceTime(inGameTime);
		m_transport->AdvanceTime(inGameTime);

		m_transport->ReadPackets();
		ReceivePackets();

		CheckForTimeOut();

		if (IsConnected())
		{
			ReceiveMessages();
		}

		SendPackets();
		m_transport->WritePackets();
	}

	eastl::weak_ptr<ONetworkPlayer> UNetworkClient::GetPlayerByID(NetworkPlayerID inID) const
	{
		auto iter = m_players.find(inID);
		return (iter != m_players.end()) ? iter->second : nullptr;
	}

	void UNetworkClient::ReceiveMessages()
	{
		MAD_ASSERT_DESC(IsConnected(), "Need a connected client to call this");

		while (auto msg = ReceiveMsg())
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
			case INITIALIZE_NEW_PLAYER:
				MInitializeNewPlayer* message = static_cast<MInitializeNewPlayer*>(msg);
				for (auto playerID : message->m_otherPlayers)
				{
					OnRemotePlayerConnected(playerID);
				}
				break;
			}

			ReleaseMsg(msg);
		}
	}

	void UNetworkClient::SetPlayerID(eastl::shared_ptr<ONetworkPlayer> inPlayer, NetworkPlayerID inPlayerID, bool inIsLocalPlayer)
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

	void UNetworkClient::OnConnected()
	{
		LOG(LogNetworkClient, Log, "[OnConnected] Client connected as client %d\n", GetClientIndex());
		SetPlayerID(m_localPlayer.lock(), GetClientIndex(), true);
	}

	void UNetworkClient::OnDisconnected()
	{
		LOG(LogNetworkClient, Log, "[OnDisconnected] Client disconnected\n");
		auto localPlayer = m_localPlayer.lock();
		m_players.clear();
		SetPlayerID(localPlayer, InvalidPlayerID, true);
	}

	void UNetworkClient::OnRemotePlayerConnected(NetworkPlayerID inNewPlayerID)
	{
		LOG(LogNetworkClient, Log, "[OnRemotePlayerConnected] Remote player connected with ID %d\n", inNewPlayerID);
		auto newPlayer = CreateDefaultObject<ONetworkPlayer>(nullptr);
		SetPlayerID(newPlayer, inNewPlayerID, false);
	}

	void UNetworkClient::OnRemotePlayerDisconnected(NetworkPlayerID inPlayerID)
	{
		MAD_ASSERT_DESC(m_localPlayer.expired() || m_localPlayer.lock()->GetPlayerID() != inPlayerID, "The remote disconnected player cannot be this local player");

		auto erased = m_players.erase(inPlayerID);
		if (!erased)
		{
			LOG(LogNetworkClient, Warning, "[OnRemotePlayerDisconnected] Remote player with ID %d not found\n", inPlayerID);
		}
		else
		{
			LOG(LogNetworkClient, Log, "[OnRemotePlayerDisconnected] Remote player %d disconnected\n", inPlayerID);
		}
	}

	void UNetworkClient::OnConnect(const Address& address)
	{
		char addressString[MaxAddressLength];
		address.ToString(addressString, sizeof(addressString));
		LOG(LogNetworkClient, Log, "[OnConnect] Client (ID=%" PRIx64 ") connecting to %s...\n", GetClientId(), addressString);

		// Create local player
		eastl::shared_ptr<ONetworkPlayer> localPlayer = CreateDefaultObject<ONetworkPlayer>(nullptr);
		SetPlayerID(localPlayer, InvalidPlayerID, true);
	}

	void UNetworkClient::OnClientStateChange(int previousState, int currentState)
	{
		(void)previousState;

		MAD_ASSERT_DESC(previousState != currentState, "This should only be called when a client's state actually changes");

		LOG(LogNetworkClient, Log, "[OnClientStateChange] Client changed state from '%s' to '%s'\n", GetClientStateName(previousState), GetClientStateName(currentState));

		if (currentState == CLIENT_STATE_CONNECTED)
		{
			OnConnected();
		}
		else if (currentState <= CLIENT_STATE_DISCONNECTED)
		{
			OnDisconnected();
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