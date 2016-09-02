#include "Core/ObjectTypeInfo.h"

namespace MAD
{
	TypeID TTypeInfo::s_currentTypeID = 0;

	TTypeInfo::TTypeInfo(const TTypeInfo* inParent) : m_typeID(s_currentTypeID++), m_parent(inParent) {}
}
