#include "Core/Object.h"

#include "Core/GameInput.h"

namespace MAD
{
	ObjectID_t UObject::s_objectRunningUID = 0;

	UObject::UObject(OGameWorld* inOwningGameWorld)
		: m_objectID(++s_objectRunningUID)
		, m_owningGameWorld(inOwningGameWorld)
		, m_netRole(ENetRole::None)
		, m_netOwner(nullptr)
		, m_isDestroyed(false) { }

	UObject::~UObject()
	{
		UGameInput::Get().UnBindObject(this);
	}

	void UObject::Destroy()
	{
		m_isDestroyed = true;
		m_objectID = InvalidObjectID;
		m_netID.Invalidate();
		m_owningGameWorld = nullptr;

		OnDestroy();
	}

	void UObject::SetNetIdentity(SNetworkID inNetID, ENetRole::Type inNetRole, class ONetworkPlayer* inNetOwner)
	{
		MAD_ASSERT_DESC(inNetID.IsValid(), "Cannot set an invalid net ID");
		MAD_ASSERT_DESC(!m_netID.IsValid(), "Cannot set a new network ID after one has already been set");

		MAD_ASSERT_DESC(inNetRole != ENetRole::None, "Cannot set a net identity with None net role");
		MAD_ASSERT_DESC(m_netRole == ENetRole::None, "Cannot set a new net role after one has already been set");

		MAD_ASSERT_DESC(inNetOwner != nullptr, "inNetOwner cannot be null");
		MAD_ASSERT_DESC(inNetOwner->GetPlayerID() != InvalidPlayerID, "Cannot set an invalid player as a net owner");
		MAD_ASSERT_DESC(m_netOwner == nullptr, "Cannot set a new net owner after this one has already been set");

		m_netID = inNetID;
		m_netRole = inNetRole;
		m_netOwner = inNetOwner;
	}

	ENetMode UObject::GetNetMode() const
	{
		return gEngine->GetNetworkManager().GetNetMode();
	}
}
