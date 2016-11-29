#include "Networking/NetworkServer.h"

#include <time.h>

#include "Core/GameEngine.h"
#include "Core/TestCharacters.h"
#include "Misc/Logging.h"
#include "Networking/NetworkManager.h"

using namespace yojimbo;

namespace MAD
{
	DECLARE_LOG_CATEGORY(LogNetworkServer);

	UNetworkServer::UNetworkServer(UNetworkManager& inNetworkManager, eastl::unique_ptr<UNetworkTransport> inServerTransport, double inCurrentGameTime, const yojimbo::ClientServerConfig& inConfig)
		: Server(GetDefaultAllocator(), *inServerTransport, inConfig, inCurrentGameTime)
		, m_networkManager(inNetworkManager)
		, m_serverTransport(eastl::move(inServerTransport))
		, m_nextNetworkID(0) { }

	void UNetworkServer::Tick(float inGameTime)
	{
		AdvanceTime(inGameTime);
		m_serverTransport->AdvanceTime(inGameTime);

		m_serverTransport->ReadPackets();
		ReceivePackets();
		ReceiveMessages();

		CheckForTimeOut();

		// TODO Temporary testing
		{
			static int nextTime = 0;

			int time = static_cast<int>(inGameTime);
			if (time >= nextTime)
			{
				nextTime = time + 5;

				// Network-spawn a point light bullet
				auto world = gEngine->GetWorld(0);
				SpawnNetworkEntity<Test::APointLightBullet>(world.get(), "default");
			}
		}

		SendNetworkStateUpdates();

		SendPackets();
		m_serverTransport->WritePackets();
	}

	eastl::weak_ptr<ONetworkPlayer> UNetworkServer::GetPlayerByID(NetworkPlayerID inID) const
	{
		auto iter = m_players.find(inID);
		return (iter != m_players.end()) ? iter->second : nullptr;
	}

	eastl::shared_ptr<UObject> UNetworkServer::GetNetworkObject(SNetworkID inNetworkID) const
	{
		auto iter = m_netObjects.find(inNetworkID);
		if (iter == m_netObjects.end())
		{
			return nullptr;
		}

		return iter->second.Object;
	}

	void UNetworkServer::SendNetworkStateUpdates()
	{
		for (auto& netObject : m_netObjects)
		{
			auto  object = netObject.second.Object;
			auto  state  = netObject.second.State;
			auto& views  = netObject.second.NetworkViews;

			for (auto player : m_players)
			{
				auto playerID = player.first;

				// Don't bother sending state updates to local players (Listen servers)
				if (player.second->IsLocalPlayer()) continue;

				// TODO If object should be visible but does not have a network view, create one for this player and send Create message
				// TODO If object should not be visible and is currently visible, remove the network view for this player and send Destroy message

				// If object should be visible and should remain visible, serialize state for this player's network view into State message
				auto view = views.find(playerID);
				if (view != views.end())
				{
					auto msg = static_cast<MUpdateObject*>(CreateMsg(playerID, UPDATE_OBJECT));
					msg->m_objectNetID = object->GetNetID();
					state->SerializeState(msg->m_networkState, false);
					SendMsg(playerID, msg);
				}
			}
		}
	}

	void UNetworkServer::ReceiveMessages()
	{
		for (const auto& player : m_players)
		{
			ReceiveMessagesForPlayer(player.first);
		}
	}

	void UNetworkServer::ReceiveMessagesForPlayer(NetworkPlayerID inPlayerID)
	{
		while (auto msg = ReceiveMsg(inPlayerID))
		{
			/*switch (msg->GetType())
			{
			default:
				LOG(LogNetworkServer, Warning, "[ReceiveMessagesForPlayer] Received unhandled or unknown message from player %d. Type: %i\n", inPlayerID, msg->GetType());
				break;
			}*/

			ReleaseMsg(inPlayerID, msg);
		}
	}

	void UNetworkServer::AddNewNetworkPlayer(NetworkPlayerID inNewPlayerID)
	{
		auto newPlayer = CreateDefaultObject<ONetworkPlayer>(nullptr);
		SetPlayerID(newPlayer, inNewPlayerID);
	}

	void UNetworkServer::RemoveNetworkPlayer(NetworkPlayerID inPlayerID)
	{
		MAD_ASSERT_DESC(m_players.find(inPlayerID) != m_players.end(), "Cannot remove player ID that does not exist in m_players");

		// Clean up all their network views
		for (auto& netObject : m_netObjects)
		{
			netObject.second.NetworkViews.erase(inPlayerID);
		}

		m_players.erase(inPlayerID);
	}

	void UNetworkServer::SetPlayerID(eastl::shared_ptr<ONetworkPlayer> inPlayer, NetworkPlayerID inPlayerID)
	{
		MAD_ASSERT_DESC(m_players.find(inPlayerID) == m_players.end(), "Cannot assign a player to an already mapped playerID");

		auto iter = m_players.find(inPlayer->GetPlayerID());
		if (iter != m_players.end())
		{
			m_players.erase(iter);
		}

		inPlayer->SetPlayerID(inPlayerID);
		inPlayer->SetIsLocalPlayer(false);

		m_players.insert({ inPlayerID, inPlayer });
	}

	eastl::shared_ptr<UObject> UNetworkServer::SpawnNetworkObject_Internal(const TTypeInfo& inTypeInfo)
	{
		SNetworkID netID;
		netID.GetUnderlyingHandleRef() = m_nextNetworkID++;

		LOG(LogNetworkServer, Log, "Spawning a UObject<%s> on the server with ID %d\n", inTypeInfo.GetTypeName(), netID.GetUnderlyingHandleRef());

		eastl::shared_ptr<UObject> object = CreateDefaultObject<UObject>(inTypeInfo, nullptr);
		object->SetNetID(netID);

		NetworkSpawn(object, inTypeInfo);

		return object;
	}

	eastl::shared_ptr<UObject> UNetworkServer::SpawnNetworkEntity_Internal(const TTypeInfo& inTypeInfo, OGameWorld* inOwningGameWorld, const eastl::string& inWorldLayer)
	{
		MAD_ASSERT_DESC(!!inOwningGameWorld, "Spawning a networked Entity requires a target game world");
		if (!inOwningGameWorld) return nullptr;

		SNetworkID netID;
		netID.GetUnderlyingHandleRef() = m_nextNetworkID++;

		LOG(LogNetworkServer, Log, "Spawning a AEntity<%s> on the server with ID %d\n", inTypeInfo.GetTypeName(), netID.GetUnderlyingHandleRef());

		auto entity = inOwningGameWorld->SpawnEntityDeferred<AEntity>(inTypeInfo, inWorldLayer);
		entity->SetNetID(netID);
		inOwningGameWorld->FinalizeSpawnEntity(entity);

		NetworkSpawn(entity, inTypeInfo);

		return entity;
	}

	void UNetworkServer::NetworkSpawn(eastl::shared_ptr<UObject> inObject, const TTypeInfo& inTypeInfo)
	{
		UNetObject netObject;
		netObject.Object = inObject;
		netObject.State = eastl::make_shared<UNetworkState>();
		netObject.State->TargetObject(inObject.get(), true);

		for (const auto& player : m_players)
		{
			auto msg = static_cast<MCreateObject*>(CreateMsg(player.first, CREATE_OBJECT));
			msg->m_objectNetID = inObject->GetNetID();
			msg->m_classTypeID = inTypeInfo.GetTypeID();

			if (auto entity = Cast<AEntity>(inObject.get()))
			{
				msg->m_worldName = entity->GetOwningWorld()->GetWorldName();
				msg->m_layerName = entity->GetOwningWorldLayer().GetLayerName();
			}

			if (!player.second->IsLocalPlayer())
			{
				// Don't bother sending state data to a local player (for Listen servers)
				netObject.State->SerializeState(msg->m_networkState, false);
			}

			SendMsg(player.first, msg);

			netObject.NetworkViews.insert({ player.first, eastl::make_shared<UNetworkObjectView>(*netObject.State) });
		}

		m_netObjects.insert({ inObject->GetNetID(), netObject });
	}

	void UNetworkServer::DestroyNetworkObject(UObject& inObject)
	{
		SNetworkID netID = inObject.GetNetID();
		if (!netID.IsValid())
		{
			LOG(LogNetworkServer, Warning, "Cannot destroy a non-networked object\n");
			return;
		}

		auto netObject = m_netObjects.find(netID);
		MAD_ASSERT_DESC(netObject != m_netObjects.end(), "An object with valid network ID must exist in the server's map");

		LOG(LogNetworkServer, Log, "Destroying server object of type %s with ID %d...\n", inObject.GetTypeInfo()->GetTypeName(), netID.GetUnderlyingHandleRef());

		for (const auto& player : m_players)
		{
			auto msg = static_cast<MDestroyObject*>(CreateMsg(player.first, DESTROY_OBJECT));
			msg->m_objectNetID = netID;
			SendMsg(player.first, msg);
		}

		inObject.Destroy();
		m_netObjects.erase(netObject);
	}

	void UNetworkServer::OnStart(int maxClients)
	{
		(void)maxClients;
		LOG(LogNetworkServer, Log, "[OnStart] MaxClients=%i\n", maxClients);
	}

	void UNetworkServer::OnStop()
	{
		LOG(LogNetworkServer, Log, "[OnStop]\n");
	}

	void UNetworkServer::OnConnectionRequest(ServerConnectionRequestAction action, const ConnectionRequestPacket& packet, const Address& address, const ConnectToken& connectToken)
	{
		(void)packet;
		(void)connectToken;

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
		(void)challengeToken;

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

		AddNewNetworkPlayer(clientIndex);

		MInitializeNewPlayer* initMsg = static_cast<MInitializeNewPlayer*>(CreateMsg(clientIndex, INITIALIZE_NEW_PLAYER));

		// Notify other clients about this new client
		for (auto iter = m_players.cbegin(); iter != m_players.cend(); ++iter)
		{
			auto idx = iter->first;
			if (idx != clientIndex)
			{
				MOtherPlayerConnectionChanged* connectMsg = static_cast<MOtherPlayerConnectionChanged*>(CreateMsg(idx, OTHER_PLAYER_CONNECTION_CHANGED));
				connectMsg->m_connect = true;
				connectMsg->m_playerID = clientIndex;
				SendMsg(idx, connectMsg);

				initMsg->m_otherPlayers.push_back(idx);
			}
		}

		// Tell this new client about other existing clients on the server
		SendMsg(clientIndex, initMsg);
	}

	void UNetworkServer::OnClientDisconnect(int clientIndex)
	{
		char addressString[MaxAddressLength];
		GetClientAddress(clientIndex).ToString(addressString, sizeof(addressString));
		LOG(LogNetworkServer, Log, "[OnClientDisconnect] Client %d disconnected (client address = %s, client id = %.16" PRIx64 ")\n", clientIndex, addressString, GetClientId(clientIndex));

		RemoveNetworkPlayer(clientIndex);
		MAD_ASSERT_DESC(m_players.find(clientIndex) == m_players.end(), "Disconnected client should no longer exist");

		for (auto iter = m_players.cbegin(); iter != m_players.cend(); ++iter)
		{
			int idx = iter->first;

			MOtherPlayerConnectionChanged* msg = static_cast<MOtherPlayerConnectionChanged*>(CreateMsg(idx, OTHER_PLAYER_CONNECTION_CHANGED));
			msg->m_connect = false;
			msg->m_playerID = clientIndex;
			SendMsg(idx, msg);
		}
	}

	void UNetworkServer::OnClientError(int clientIndex, ServerClientError error)
	{
		(void)clientIndex;
		(void)error;

		switch (error)
		{
		case SERVER_CLIENT_ERROR_TIMEOUT:
			LOG(LogNetworkServer, Warning, "[OnClientError] Client %d timed out (client id = %.16" PRIx64 ")\n", clientIndex, GetClientId(clientIndex));
			break;

		case SERVER_CLIENT_ERROR_ALLOCATOR:
			LOG(LogNetworkServer, Warning, "[OnClientError] Client %d experienced an allocator error (client id = %.16" PRIx64 ")\n", clientIndex, GetClientId(clientIndex));
			break;

		case SERVER_CLIENT_ERROR_CONNECTION:
			LOG(LogNetworkServer, Warning, "[OnClientError] Client %d experienced a connection error (client id = %.16" PRIx64 ")\n", clientIndex, GetClientId(clientIndex));
			break;

		case SERVER_CLIENT_ERROR_MESSAGE_FACTORY:
			LOG(LogNetworkServer, Warning, "[OnClientError] Client %d experienced a message factory error (client id = %.16" PRIx64 ")\n", clientIndex, GetClientId(clientIndex));
			break;

		case SERVER_CLIENT_ERROR_PACKET_FACTORY:
			LOG(LogNetworkServer, Warning, "[OnClientError] Client %d experienced a packet factory error (client id = %.16" PRIx64 ")\n", clientIndex, GetClientId(clientIndex));
			break;

		default:
			break;
		}
	}

	void UNetworkServer::OnPacketSent(int packetType, const Address& to, bool immediate)
	{
		(void)immediate;

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
		to.ToString(addressString, sizeof(addressString));
		LOG(LogNetworkServer, Log, "[OnPacketSent] Server sent `%s` packet to %s%s\n", packetTypeString, addressString, immediate ? " (immediate)" : "");
#endif
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

#if NETWORK_VERBOSE_LOGGING
		char addressString[MaxAddressLength];
		from.ToString(addressString, sizeof(addressString));
		LOG(LogNetworkServer, Log, "[OnPacketReceived] Server received `%s` packet from %s\n", packetTypeString, addressString);
#endif
	}
}
