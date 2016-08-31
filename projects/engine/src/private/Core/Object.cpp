#include "Core/Object.h"

namespace MAD
{
	UObject::UObject() : m_objectID(s_objectRunningUID++) {}
}
