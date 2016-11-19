#include "Core/Object.h"

#include "Core/GameInput.h"

namespace MAD
{
	ObjectID UObject::s_objectRunningUID = 0;

	UObject::UObject(OGameWorld* inOwningGameWorld)
		: m_objectID(++s_objectRunningUID)
		, m_owningGameWorld(inOwningGameWorld) {}

	UObject::~UObject()
	{
		UGameInput::Get().UnBindObject(this);
	}

	void UObject::SetNetID(SNetworkID inNetID)
	{
		MAD_ASSERT_DESC(inNetID.IsValid(), "Cannot set an invalid net ID");
		MAD_ASSERT_DESC(!m_netID.IsValid(), "Cannot set a new network ID after one has already been set");
		m_netID = inNetID;
	}
}
