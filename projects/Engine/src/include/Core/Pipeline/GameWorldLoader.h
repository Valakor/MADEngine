#pragma once

#include <EASTL/shared_ptr.h>
#include <EASTl/stack.h>
#include <EASTL/string.h>

#include "JSONTypes.h"

namespace MAD
{
	class OGameWorld;
	
	class UGameWorldLoader
	{
	public:
		bool LoadWorld(const eastl::string& inWorldFilePath);
	private:
		rapidjson::Document m_doc;

		eastl::string m_relativeFilePath;
		eastl::string m_fullFilePath;

		eastl::shared_ptr<class OGameWorld> m_world;

		bool LoadWorld(UObjectValue& inWorld);
		bool LoadLayer(UObjectValue& inLayer);
		bool LoadEntity(UObjectValue& inEntity, const eastl::string& inLayerName);
		bool LoadExistingComponent(UObjectValue& inExistingComp, eastl::shared_ptr<class AEntity> inOwningEntity);
		bool LoadNewComponent(UObjectValue& inNewComp, eastl::shared_ptr<class AEntity> inOwningEntity);
	};
}
