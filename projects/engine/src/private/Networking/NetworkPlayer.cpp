#include "Networking/NetworkPlayer.h"

namespace MAD
{
	ONetworkPlayer::ONetworkPlayer(OGameWorld* inOwningWorld) : Super_t(inOwningWorld)
		, m_playerID(InvalidPlayerID)
		, m_isLocalPlayer(false)
	{

	}
}
