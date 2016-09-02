#include "Core/Object.h"

namespace MAD
{
	ObjectID UObject::s_objectRunningUID = 0;

	UObject::UObject() : m_objectID(s_objectRunningUID++) {}
}
