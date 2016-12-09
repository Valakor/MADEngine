#pragma once

#include <EASTL/shared_ptr.h>

#include "Core/Object.h"
#include "Networking/NetworkState.h"

namespace MAD
{
	class UNetworkObjectView
	{
	public:
		UNetworkObjectView(UNetworkState& inNetworkState);

	private:
		UNetworkState& m_state;
	};
}
