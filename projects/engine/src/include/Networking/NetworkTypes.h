#pragma once

#include <EASTL/fixed_vector.h>
#include <EASTL/shared_ptr.h>
#include <yojimbo/yojimbo.h>

#include "Core/ObjectTypeInfo.h"
#include "Networking/Network.h"
#include "Networking/NetworkState.h"
#include "Networking/NetworkObjectView.h"

namespace MAD
{
#define NETWORK_VERBOSE_LOGGING 1

#define serialize_typeID(stream, value) \
	serialize_int(stream, value, eastl::numeric_limits<TypeID>::min(), eastl::numeric_limits<TypeID>::max())

#define serialize_netID(stream, value) \
	serialize_int(stream, value.GetUnderlyingHandleRef(), eastl::numeric_limits<SNetworkID::HandleType>::min(), eastl::numeric_limits<SNetworkID::HandleType>::max())

	const size_t MaxStateUpdateSize = 252; // Needs to be 4 byte aligned for scratch memory in libyojimbo stream
#define serialize_state(stream, state) \
	do { \
		static_assert(eastl::is_same<decltype(state), eastl::vector<uint8_t>>::value, "Expected state data to be of type eastl::vector<uint8_t>"); \
		MAD_ASSERT_DESC(state.size() <= MaxStateUpdateSize, "A state update must be <= 252 bytes"); \
		int stateLength = static_cast<int>(state.size()); \
		serialize_int(stream, stateLength, 0, MaxStateUpdateSize); \
		if (Stream::IsReading) state.resize(stateLength); \
		if (stateLength > 0) serialize_bytes(stream, &state[0], stateLength); \
	} while (0)

	const size_t MaxNetworkStringLength = 127;
	template <typename Stream> bool serialize_string_internal(Stream & stream, eastl::string& string )
	{
		int length = 0;
		if (Stream::IsWriting)
		{
			length = (int)string.size();
			MAD_ASSERT_DESC(string.size() <= MaxNetworkStringLength, "Cannot send a string longer than 127 characters");
		}
		serialize_int(stream, length, 0, MaxNetworkStringLength);
		if (Stream::IsReading) string.resize(length);
		if (length > 0) serialize_bytes(stream, (uint8_t*)&string[0], length);
		return true;
	}

	#define serialize_string( stream, string )                                              \
        do                                                                                  \
        {                                                                                   \
            if ( !MAD::serialize_string_internal( stream, string ) )                        \
                return false;                                                               \
        } while (0)

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
		eastl::string m_worldName;
		eastl::string m_layerName;
		eastl::vector<uint8_t> m_networkState;

		template <typename Stream> bool Serialize(Stream & stream)
		{
			serialize_typeID(stream, m_classTypeID);
			serialize_netID(stream, m_objectNetID);
			serialize_string(stream, m_worldName);
			serialize_string(stream, m_layerName);
			serialize_state(stream, m_networkState);
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
		eastl::vector<uint8_t> m_networkState;

		template <typename Stream> bool Serialize(Stream & stream)
		{
			serialize_netID(stream, m_objectNetID);
			serialize_state(stream, m_networkState);
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
