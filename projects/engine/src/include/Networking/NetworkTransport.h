#pragma once

#include <yojimbo/yojimbo.h>

#include "Networking/Network.h"
#include "Networking/NetworkTypes.h"

namespace MAD
{
	class UNetworkTransport : public yojimbo::SocketTransport
	{
	public:

		UNetworkTransport(const yojimbo::Address& address = yojimbo::Address("0.0.0.0"))
			: SocketTransport(yojimbo::GetDefaultAllocator(), address, NETWORK_PROTOCOL_ID)
		{ }

		~UNetworkTransport();
	};
}
