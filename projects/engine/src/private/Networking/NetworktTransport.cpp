#include "Networking/NetworkTransport.h"

namespace MAD
{
	UNetworkTransport::~UNetworkTransport()
	{
		ClearSendQueue();
		ClearReceiveQueue();
	}
}
