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

		// Helpers
		bool GetFloat(const char* inProp, float& outFloat) const;
		bool GetInt(const char* inProp, int& outInt) const;
		bool GetString(const char* inProp, eastl::string& outString) const;
		bool GetBool(const char* inProp, bool& outBool) const;
		bool GetVector(const char* inProp, Vector3& outVector) const;
		bool GetColor(const char* inProp, Color& outColor) const;
		bool GetQuaternion(const char* inProp, Quaternion& outQuat) const; // Reads quat directly
		bool GetRotation(const char* inProp, Quaternion& outRot) const; // Reads quat as pitch/yaw/roll (converts degrees -> rads)
		bool GetObject(const char* inProp, UObjectValue& outObject) const;
		bool GetArray(const char* inProp, UArrayValue& outArray) const;
	private:
		rapidjson::Document m_doc;
		rapidjson::Value* m_currentValue = nullptr;

		eastl::stack<rapidjson::Value*> m_jsonValueStack;
		eastl::string m_relativeFilePath;
		eastl::string m_fullFilePath;

		eastl::shared_ptr<class OGameWorld> m_world;

		bool LoadWorld(rapidjson::Value& inRoot);
		bool LoadLayer(rapidjson::Value& inRoot);
		bool LoadEntity(rapidjson::Value& inRoot, const eastl::string& inLayerName);
		bool LoadExistingComponent(rapidjson::Value& inRoot, eastl::shared_ptr<class AEntity> inOwningEntity);
		bool LoadNewComponent(rapidjson::Value& inRoot, eastl::shared_ptr<class AEntity> inOwningEntity);
	};
}
