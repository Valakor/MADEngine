#include "Core/ObjectTypeInfo.h"

namespace MAD
{
	TypeID TTypeInfo::s_currentTypeID = 0;

	TTypeInfo::TTypeInfo(const TTypeInfo* inParent, const char* inTypeName, CreationFunction_t inCreationFunc)
		: m_creationFunction(inCreationFunc)
		, m_typeName(inTypeName)
		, m_typeID(++s_currentTypeID)
		, m_parent(inParent) {}
}
