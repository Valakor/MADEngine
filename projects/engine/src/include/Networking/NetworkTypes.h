#pragma once

#include <EASTL/fixed_vector.h>
#include <yojimbo/yojimbo.h>

#include "Core/ObjectTypeInfo.h"
#include "Networking/Network.h"

namespace MAD
{
#define NETWORK_VERBOSE_LOGGING 1

#define serialize_typeID(stream, value) \
	serialize_int(stream, value, eastl::numeric_limits<TypeID>::min(), eastl::numeric_limits<TypeID>::max())

#define serialize_netID(stream, value) \
	serialize_int(stream, value.GetUnderlyingHandleRef(), eastl::numeric_limits<SNetworkID::HandleType>::min(), eastl::numeric_limits<SNetworkID::HandleType>::max())

	// Sent to every player except the player whose connection changed
	struct MOtherPlayerConnectionChanged : public yojimbo::Message
	{
		NetworkPlayerID m_playerID;
		bool m_connect;

		MOtherPlayerConnectionChanged()
		{
			m_playerID = InvalidPlayerID;
			m_connect = false;
		}

		template <typename Stream> bool Serialize(Stream & stream)
		{
			serialize_int(stream, m_playerID, 0, yojimbo::MaxClients);
			serialize_bool(stream, m_connect);
			return true;
		}

		YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS();
	};

	// Sent to a newly-connected player
	struct MInitializeNewPlayer : public yojimbo::Message
	{
		eastl::fixed_vector<NetworkPlayerID, yojimbo::MaxClients, false> m_otherPlayers;

		MInitializeNewPlayer()
		{

		}

		template <typename Stream> bool Serialize(Stream & stream)
		{
			int numOtherPlayers = static_cast<int>(m_otherPlayers.size());
			serialize_int(stream, numOtherPlayers, 0, yojimbo::MaxClients);

			if (Stream::IsReading)
			{
				m_otherPlayers.resize(numOtherPlayers);
			}

			for (int i = 0; i < numOtherPlayers; ++i)
			{
				serialize_int(stream, m_otherPlayers[i], 0, yojimbo::MaxClients);
			}

			return true;
		}

		YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS();
	};

	// Sent to players to tell them to spawn an object
	struct MCreateObject : public yojimbo::Message
	{
		TypeID m_classTypeID;
		SNetworkID m_objectNetID;

		template <typename Stream> bool Serialize(Stream & stream)
		{
			serialize_typeID(stream, m_classTypeID);
			serialize_netID(stream, m_objectNetID);
			return true;
		}

		YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS();
	};

	// Sent to players to tell them to destroy an object
	struct MDestroyObject : public yojimbo::Message
	{
		SNetworkID m_objectNetID;

		template <typename Stream> bool Serialize(Stream & stream)
		{
			serialize_netID(stream, m_objectNetID);
			return true;
		}

		YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS();
	};

	// Sent to players to tell them to update the state of an object
	struct MUpdateObject : public yojimbo::Message
	{
		SNetworkID m_objectNetID;

		template <typename Stream> bool Serialize(Stream & stream)
		{
			serialize_netID(stream, m_objectNetID);
			return true;
		}

		YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS();
	};

	enum EMessageTypes
	{
		OTHER_PLAYER_CONNECTION_CHANGED,
		INITIALIZE_NEW_PLAYER,
		CREATE_OBJECT,
		DESTROY_OBJECT,
		UPDATE_OBJECT,

		NUM_MESSAGE_TYPES
	};

	YOJIMBO_MESSAGE_FACTORY_START(UGameMessageFactory, yojimbo::MessageFactory, NUM_MESSAGE_TYPES);
		YOJIMBO_DECLARE_MESSAGE_TYPE(OTHER_PLAYER_CONNECTION_CHANGED, MOtherPlayerConnectionChanged);
		YOJIMBO_DECLARE_MESSAGE_TYPE(INITIALIZE_NEW_PLAYER, MInitializeNewPlayer);
		YOJIMBO_DECLARE_MESSAGE_TYPE(CREATE_OBJECT, MCreateObject);
		YOJIMBO_DECLARE_MESSAGE_TYPE(DESTROY_OBJECT, MDestroyObject);
		YOJIMBO_DECLARE_MESSAGE_TYPE(UPDATE_OBJECT, MUpdateObject);
	YOJIMBO_MESSAGE_FACTORY_FINISH();
}
