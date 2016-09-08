#include "Core/Object.h"

namespace MAD
{
	ObjectID UObject::s_objectRunningUID = 0;

	UObject::UObject(OGameWorld* inOwningGameWorld /*= nullptr*/)
		: m_objectID(++s_objectRunningUID)
		, m_owningGameWorld(inOwningGameWorld) {}

}
