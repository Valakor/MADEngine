#pragma once

#include "Core/Object.h"
#include "Networking/Network.h"

namespace MAD
{
	class ONetworkPlayer : public UObject
	{
		MAD_DECLARE_CLASS(ONetworkPlayer, UObject)
	public:
		ONetworkPlayer(OGameWorld* inOwningWorld);

		NetworkPlayerID GetPlayerID() const { return m_playerID; }
		void SetPlayerID(NetworkPlayerID inPlayerID) { m_playerID = inPlayerID; }

		bool IsLocalPlayer() const { return m_isLocalPlayer; }
		void SetIsLocalPlayer(bool inIsLocalPlayer) { m_isLocalPlayer = inIsLocalPlayer; }

	private:
		NetworkPlayerID m_playerID;
		bool m_isLocalPlayer;
	};
}
