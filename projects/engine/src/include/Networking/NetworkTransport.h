#pragma once

#include <yojimbo/yojimbo.h>

#include "Networking/Network.h"
#include "Networking/NetworkTypes.h"

namespace MAD
{
	class UNetworkTransport : public yojimbo::NetworkTransport
	{
	public:

		UNetworkTransport(double inCurrentGameTime, const yojimbo::Address& address = yojimbo::Address("0.0.0.0"))
			: NetworkTransport(yojimbo::GetDefaultAllocator(), address, NETWORK_PROTOCOL_ID, inCurrentGameTime)
		{ }

		~UNetworkTransport();
	};
}
