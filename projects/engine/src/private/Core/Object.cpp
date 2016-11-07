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
}
