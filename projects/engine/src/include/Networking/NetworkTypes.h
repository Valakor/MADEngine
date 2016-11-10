#pragma once

#include <yojimbo/yojimbo.h>

namespace MAD
{
#define NETWORK_VERBOSE_LOGGING 1

	struct UEventMessage : public yojimbo::Message
	{
		uint16_t sequence;

		UEventMessage()
		{
			sequence = 0;
		}

		template <typename Stream> bool Serialize(Stream & stream)
		{
			serialize_bits(stream, sequence, 16);
			return true;
		}

		YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS();
	};

	enum EMessageTypes
	{
		EVENT_MESSAGE,
		NUM_MESSAGE_TYPES
	};

	YOJIMBO_MESSAGE_FACTORY_START(UGameMessageFactory, yojimbo::MessageFactory, NUM_MESSAGE_TYPES);
		YOJIMBO_DECLARE_MESSAGE_TYPE(EVENT_MESSAGE, UEventMessage);
	YOJIMBO_MESSAGE_FACTORY_FINISH();
}
