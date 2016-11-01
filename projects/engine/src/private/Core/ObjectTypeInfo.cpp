#include "Core/ObjectTypeInfo.h"

#include "Misc/Assert.h"
#include "Misc/Logging.h"

namespace MAD
{
	DECLARE_LOG_CATEGORY(LogTypeInfo);

	TypeID TTypeInfo::s_currentTypeID = 0;
	eastl::string_hash_map<const TTypeInfo*> TTypeInfo::s_typeNameToTypeInfoMap;

	const TTypeInfo* TTypeInfo::GetTypeInfo(const eastl::string& inTypeName)
	{
		auto iter = s_typeNameToTypeInfoMap.find(inTypeName.c_str());
		if (iter == s_typeNameToTypeInfoMap.end())
		{
			return nullptr;
		}

		return iter->second;
	}

	void TTypeInfo::DumpTypeInfo()
	{
		LOG(LogTypeInfo, Log, "Registered TTypeInfo:\n");
		for (const auto& tInfo : s_typeNameToTypeInfoMap)
		{
			LOG(LogTypeInfo, Log, "  [%i] %s\n", tInfo.second->m_typeID, tInfo.first);
			(void)tInfo;
		}
	}

	TTypeInfo::TTypeInfo(const TTypeInfo* inParent, const char* inTypeName, CreationFunction_t inCreationFunc)
		: m_creationFunction(inCreationFunc)
		, m_typeName(inTypeName)
		, m_typeID(++s_currentTypeID)
		, m_parent(inParent)
	{
		MAD_ASSERT_DESC(s_typeNameToTypeInfoMap.find(inTypeName) == s_typeNameToTypeInfoMap.end(), "A TTypeInfo can only be registered once");
		s_typeNameToTypeInfoMap.insert(inTypeName, this);
	}
}
