#pragma once

#include <EASTL/string.h>
#include <rapidjson/document.h>

#include "Core/SimpleMath.h"

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

	private:
		rapidjson::Document m_doc;
		rapidjson::Value* m_currentValue = nullptr;
	};
}
