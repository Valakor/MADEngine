#pragma once

#include "Core/SimpleMath.h"
#include "Misc/Assert.h"

#include <rapidjson/document.h>

namespace MAD
{
	class UArrayValue;
	class UObjectValue;

	using SizeType = rapidjson::SizeType;

	class UGenericValue
	{
	public:
		UGenericValue(rapidjson::Value* inValue = nullptr) : m_value(inValue) {} // Non-explicit constructor for conversion convenience

		template <typename ValueType> bool Get(ValueType& outValue) const;
		template <typename ValueType> bool IsA() const;
	private:
		friend class UObjectValue;
		friend class UArrayValue;
	private:
		rapidjson::Value* m_value;
	};

	class UObjectValue
	{
	public:
		UObjectValue() {}
		UObjectValue(const UGenericValue& inObjectValue) : m_objectValue(inObjectValue) {}

		template <typename ValueType> bool GetProperty(const char* inPropName, ValueType& outPropValue);
	private:
		UGenericValue m_objectValue;
	};

	class UArrayValue
	{
	public:
		UArrayValue() {}
		UArrayValue(const UGenericValue& inArrayValue) : m_arrayElementValue(inArrayValue) {}

		UGenericValue operator[](SizeType inIndex) const;

		SizeType Size() const;
	private:
		UGenericValue m_arrayElementValue;
	};

	template <typename ValueType>
	bool UGenericValue::Get(ValueType&) const
	{
		MAD_ASSERT_DESC(false, "Invalid type for Generic Value! Type is probably not specialized yet, include specialization below\n");
		return false;
	}

	template <typename ValueType>
	bool UGenericValue::IsA() const
	{
		MAD_ASSERT_DESC(false, "Invalid type for Generic Value! Type is probably not specialized yet, include specialization below\n");
		return false;
	}

	template <typename ValueType>
	bool UObjectValue::GetProperty(const char* inPropName, ValueType& outPropValue)
	{
		auto propFindIter = m_objectValue.m_value->FindMember(inPropName);

		if (propFindIter == m_objectValue.m_value->MemberEnd())
		{
			return false;
		}

		UGenericValue propertyValue(&propFindIter->value);

		return propertyValue.Get(outPropValue);
	}

	// Note: The Standard says that you have to place explicit specialization declarations OUTSIDE of the enclosing class

	// Explicit specializations for UGenericValue
	template <> bool UGenericValue::Get<float>(float& outFloat) const;
	template <> bool UGenericValue::IsA<float>() const;
	template <> bool UGenericValue::IsA<double>() const;

	template <> bool UGenericValue::Get<uint32_t>(uint32_t& outUInt) const;
	template <> bool UGenericValue::IsA<uint32_t>() const;

	template <> bool UGenericValue::Get<int32_t>(int32_t& outInt) const;
	template <> bool UGenericValue::IsA<int32_t>() const;

	template <> bool UGenericValue::Get<eastl::string>(eastl::string& outString) const;
	template <> bool UGenericValue::IsA<eastl::string>() const;

	template <> bool UGenericValue::Get<bool>(bool& outBool) const;
	template <> bool UGenericValue::IsA<bool>() const;

	template <> bool UGenericValue::Get<Vector3>(Vector3& outVector3) const;
	template <> bool UGenericValue::IsA<Vector3>() const;

	template <> bool UGenericValue::Get<Color>(Color& outColor) const;
	template <> bool UGenericValue::Get<Quaternion>(Quaternion& outQuaternion) const;
	template <> bool UGenericValue::Get<UObjectValue>(UObjectValue& outObjectValue) const;
	template <> bool UGenericValue::Get<UArrayValue>(UArrayValue& outArrayValue) const;

	// Explicit specializations for UObjectValue
	template <> bool UObjectValue::GetProperty<UObjectValue>(const char* inPropName, UObjectValue& outObjectValue);
	template <> bool UObjectValue::GetProperty<UArrayValue>(const char* inPropName, UArrayValue& outArrayValue);
}