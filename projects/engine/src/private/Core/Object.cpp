#include "Core\Object.h"

namespace MAD
{
	MAD_IMPLEMENT_BASE_CLASS(UObject)

	UObject::UObject() : m_objectID(s_objectRunningUID++) {}
}