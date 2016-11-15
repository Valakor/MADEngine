#pragma once

#include <EASTL/fixed_vector.h>
#include <yojimbo/yojimbo.h>

#include "Networking/Network.h"

namespace MAD
{
#define NETWORK_VERBOSE_LOGGING 1

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
			serialize_int(stream, m_playerID, 0, 64);
			serialize_bool(stream, m_connect);
			return true;
		}

		YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS();
	};

	struct MInitializeNewPlayer : public yojimbo::Message
	{
		eastl::fixed_vector<NetworkPlayerID, yojimbo::MaxClients, false> m_otherPlayers;

		MInitializeNewPlayer()
		{

		}

		template <typename Stream> bool Serialize(Stream & stream)
		{
			int numOtherPlayers = static_cast<int>(m_otherPlayers.size());
			serialize_int(stream, numOtherPlayers, 0, 64);

			if (Stream::IsReading)
			{
				m_otherPlayers.resize(numOtherPlayers);
			}

			for (int i = 0; i < numOtherPlayers; ++i)
			{
				serialize_int(stream, m_otherPlayers[i], 0, 64);
			}

			return true;
		}

		YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS();
	};

	enum EMessageTypes
	{
		OTHER_PLAYER_CONNECTION_CHANGED,
		INITIALIZE_NEW_PLAYER,
		NUM_MESSAGE_TYPES
	};

	YOJIMBO_MESSAGE_FACTORY_START(UGameMessageFactory, yojimbo::MessageFactory, NUM_MESSAGE_TYPES);
		YOJIMBO_DECLARE_MESSAGE_TYPE(OTHER_PLAYER_CONNECTION_CHANGED, MOtherPlayerConnectionChanged);
		YOJIMBO_DECLARE_MESSAGE_TYPE(INITIALIZE_NEW_PLAYER, MInitializeNewPlayer);
	YOJIMBO_MESSAGE_FACTORY_FINISH();
}
