#include "Networking/NetworkPlayer.h"

namespace MAD
{
	ONetworkPlayer::ONetworkPlayer(OGameWorld* inOwningWorld) : Super(inOwningWorld)
		, m_playerID(InvalidPlayerID)
		, m_isLocalPlayer(false)
	{

	}
}
