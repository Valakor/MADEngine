#pragma once

#include "Core/Object.h"

namespace MAD
{
	class ONetworkPlayer : public UObject
	{
		MAD_DECLARE_CLASS(ONetworkPlayer, UObject)
	public:

		ONetworkPlayer(OGameWorld* inOwningWorld);
	private:
	};
}
