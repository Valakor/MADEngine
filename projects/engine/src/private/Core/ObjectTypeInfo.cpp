#include "Core\ObjectTypeInfo.h"

namespace MAD
{
	TTypeInfo::TTypeInfo(const TTypeInfo* inParent) : m_parent(inParent) {}
}