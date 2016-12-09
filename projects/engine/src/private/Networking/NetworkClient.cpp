#include "Networking/NetworkClient.h"

#include "Core/GameEngine.h"
#include "Misc/Logging.h"
#include <complex.h>

using namespace yojimbo;

namespace MAD
{
	DECLARE_LOG_CATEGORY(LogNetworkClient);

	UNetworkClient::UNetworkClient(UNetworkManager& inNetworkManager, eastl::unique_ptr<UNetworkTransport> inClientTransport, double inCurrentGameTime, const ClientServerConfig& inClientConfig)
		: Client(GetDefaultAllocator(), *inClientTransport, inClientConfig, inCurrentGameTime)
		, m_networkManager(inNetworkManager)
		, m_clientTransport(eastl::move(inClientTransport))
	{

	}

	void UNetworkClient::PreTick(double inGameTime)
	{
		AdvanceTime(inGameTime);
		m_transport->AdvanceTime(inGameTime);

		m_transport->ReadPackets();
		ReceivePackets();

		if (IsConnected())
		{
			ReceiveMessages();
		}

		CheckForTimeOut();
	}

	void UNetworkClient::PostTick()
	{
		SendPackets();
		m_transport->WritePackets();
	}

	eastl::weak_ptr<ONetworkPlayer> UNetworkClient::GetPlayerByID(NetworkPlayerID inID) const
	{
		auto iter = m_players.find(inID);
		return (iter != m_players.end()) ? iter->second : nullptr;
	}

	void UNetworkClient::SendNetworkEvent(EEventTypes inEventType, UObject& inTargetObject, void* inEventData, size_t inEventSize)
	{
		if (!IsConnected())
		{
			LOG(LogNetworkClient, Warning, "Trying to send a network event while not connected to server\n");
			return;
		}

		auto msg = static_cast<MEvent*>(CreateMsg(EVENT));

		msg->m_eventType = inEventType;
		msg->m_targetObjectID = inTargetObject.GetNetID();
		msg->m_eventData.resize(inEventSize);
		memcpy(msg->m_eventData.data(), inEventData, inEventSize);

		SendMsg(msg, MEvent::MessageChannel);
	}

	void UNetworkClient::ReceiveMessages()
	{
		MAD_ASSERT_DESC(IsConnected(), "Need a connected client to call this");

		int numChannels = m_config.connectionConfig.numChannels;

		for (int channelID = 0; channelID < numChannels; ++channelID)
		{
			ReceiveMessagesForChannel(channelID);
		}
	}

	void UNetworkClient::ReceiveMessagesForChannel(int inChannelID)
	{
		while (auto msg = ReceiveMsg(inChannelID))
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
			{
				MInitializeNewPlayer* message = static_cast<MInitializeNewPlayer*>(msg);
				for (auto playerID : message->m_otherPlayers)
				{
					OnRemotePlayerConnected(playerID);
				}
				break;
			}

			case CREATE_OBJECT:
			{
				MCreateObject* message = static_cast<MCreateObject*>(msg);
				HandleCreateObjectMessage(*message);
				break;
			}

			case UPDATE_OBJECT:
			{
				MUpdateObject* message = static_cast<MUpdateObject*>(msg);
				HandleUpdateObjectMessage(*message);
				break;
			}

			case DESTROY_OBJECT:
			{
				MDestroyObject* message = static_cast<MDestroyObject*>(msg);
				HandleDestroyObjectMessage(*message);
				break;
			}

			case EVENT:
			{
				MEvent* message = static_cast<MEvent*>(msg);
				HandleEventMessage(*message);
				break;
			}

			default:
				LOG(LogNetworkClient, Warning, "[ReceiveMessages] Received unhandled or unknown message type: %i\n", msg->GetType());
				break;
			}

			ReleaseMsg(msg);
		}
	}

	void UNetworkClient::HandleCreateObjectMessage(MCreateObject& message)
	{
		const TTypeInfo* objTypeInfo = TTypeInfo::GetTypeInfo(message.m_classTypeID);
		MAD_ASSERT_DESC(!!objTypeInfo, "Received invalid Type Info");

		UNetObject netObject;
		netObject.State = eastl::make_shared<UNetworkState>();
		netObject.NetworkView = eastl::make_shared<UNetworkObjectView>(*netObject.State);

		if (m_networkManager.GetNetMode() == ENetMode::ListenServer)
		{
			// Don't create the object again, find it in the server object
			MAD_ASSERT_DESC(m_networkManager.m_server != nullptr, "Must have a valid NetworkServer if in ListenServer mode");
			auto serverObject = m_networkManager.m_server->GetNetworkObject(message.m_objectNetID);
			MAD_ASSERT_DESC(serverObject != nullptr, "Server should have an object with this ID already");
			netObject.Object = serverObject;
			netObject.State->TargetObject(netObject.Object.get(), false);
		}
		else
		{
			auto netOwner = GetPlayerByID(message.m_netOwnerID).lock();
			MAD_ASSERT_DESC(netOwner != nullptr, "Can only spawn objects for players that the Client knows about");

			auto netRole = (netOwner->GetPlayerID() == m_localPlayer.lock()->GetPlayerID()) ? ENetRole::Authority_Proxy : ENetRole::Simulated_Proxy;

			if (IsA<AEntity>(*objTypeInfo))
			{
				auto world = gEngine->GetWorld(message.m_worldName);
				if (!world)
				{
					LOG(LogNetworkClient, Error, "Client should have a world loaded with the given name: %s\n", message.m_worldName.c_str());
					return;
				}

				LOG(LogNetworkClient, Log, "Spawning a AEntity<%s> on the client with ID %d\n", objTypeInfo->GetTypeName(), message.m_objectNetID.GetUnderlyingHandle());

				auto entity = world->SpawnEntityDeferred<AEntity>(*objTypeInfo, message.m_layerName);
				entity->SetNetIdentity(message.m_objectNetID, netRole, netOwner.get());
				netObject.State->TargetObject(entity.get(), false);
				netObject.State->SerializeState(message.m_networkState, true);
				world->FinalizeSpawnEntity(entity);

				netObject.Object = entity;
			}
			else
			{
				LOG(LogNetworkClient, Log, "Spawning a UObject<%s> on the client with ID %d\n", objTypeInfo->GetTypeName(), message.m_objectNetID.GetUnderlyingHandle());

				netObject.Object = CreateDefaultObject<UObject>(*objTypeInfo, nullptr);
				netObject.Object->SetNetIdentity(message.m_objectNetID, netRole, netOwner.get());
				netObject.State->TargetObject(netObject.Object.get(), false);
				netObject.State->SerializeState(message.m_networkState, true);
			}
		}

		m_netObjects.insert({ message.m_objectNetID, netObject });
	}

	void UNetworkClient::HandleUpdateObjectMessage(MUpdateObject& message)
	{
		MAD_ASSERT_DESC(m_networkManager.GetNetMode() != ENetMode::ListenServer, "Server shouldn't send this message to its local client");

		auto netObject = m_netObjects.find(message.m_objectNetID);
		if (netObject == m_netObjects.end())
		{
			LOG(LogNetworkClient, Warning, "Received UpdateObject message for unrecognized object ID: %i\n", message.m_objectNetID.GetUnderlyingHandle());
			return;
		}

		netObject->second.State->SerializeState(message.m_networkState, true);
	}

	void UNetworkClient::HandleDestroyObjectMessage(MDestroyObject& message)
	{
		auto netObject = m_netObjects.find(message.m_objectNetID);
		if (netObject == m_netObjects.end())
		{
			LOG(LogNetworkClient, Warning, "Received DestroyObject message for unrecognized object ID: %i\n", message.m_objectNetID.GetUnderlyingHandle());
			return;
		}

		auto object = netObject->second.Object;
		LOG(LogNetworkClient, Log, "Destroying a client object of type %s with ID %d\n", object->GetTypeInfo()->GetTypeName(), object->GetNetID().GetUnderlyingHandle());
		
		if (m_networkManager.GetNetMode() != ENetMode::ListenServer)
		{
			// Destroy is called by the Server already for a listen server
			object->Destroy();
		}

		m_netObjects.erase(netObject);
	}

	void UNetworkClient::HandleEventMessage(MEvent& message)
	{
		auto targetObject = m_netObjects.find(message.m_targetObjectID);

		if (targetObject == m_netObjects.end())
		{
			LOG(LogNetworkClient, Warning, "Received event for invalid target network object!\n");
			return;
		}

		targetObject->second.Object->OnEvent(message.m_eventType, message.m_eventData.data());
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
		LOG(LogNetworkClient, Log, "[OnDisconnected] Client disconnected. Cleaning up net objects and player\n");
		auto localPlayer = m_localPlayer.lock();

		// Clean up all networked objects
		for (auto& netObject : m_netObjects)
		{
			if (m_networkManager.GetNetMode() != ENetMode::ListenServer)
			{
				netObject.second.Object->Destroy();
			}
		}

		m_netObjects.clear();

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

		auto iter = m_players.find(inPlayerID);
		if (iter != m_players.end())
		{
			LOG(LogNetworkClient, Log, "[OnRemotePlayerDisconnected] Remote player %d disconnected\n", inPlayerID);
			DestroyNetObjectsForRemotePlayer(*iter->second);
			m_players.erase(iter);
		}
		else
		{
			LOG(LogNetworkClient, Warning, "[OnRemotePlayerDisconnected] Remote player with ID %d not found\n", inPlayerID);
		}
	}

	void UNetworkClient::DestroyNetObjectsForRemotePlayer(const ONetworkPlayer& inPlayer)
	{
		if (m_networkManager.GetNetMode() == ENetMode::ListenServer)
		{
			// Server code will have already called Destroy appropriately
			return;
		}

		auto iter = m_netObjects.begin();
		while (iter != m_netObjects.end())
		{
			auto object = iter->second.Object;
			if (object->GetNetOwner()->GetPlayerID() == inPlayer.GetPlayerID())
			{
				object->Destroy();
				iter = m_netObjects.erase(iter);
			}
			else
			{
				++iter;
			}
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