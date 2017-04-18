#include "Core/Pipeline/JSONTypes.h"

namespace MAD
{
	template <>
	bool UGenericValue::Get(float& outFloat) const
	{
		if (!m_value->IsFloat())
		{
			return false;
		}

		outFloat = m_value->GetFloat();
		return true;
	}

	template <>
	bool UGenericValue::IsA<float>() const
	{
		return m_value->IsFloat();
	}

	template <>
	bool UGenericValue::IsA<double>() const
	{
		return m_value->IsDouble();
	}

	template <>
	bool UGenericValue::Get(uint32_t& outUInt) const
	{
		if (!m_value->IsUint())
		{
			return false;
		}

		outUInt = m_value->GetUint();
		return true;
	}

	template <>
	bool UGenericValue::IsA<uint32_t>() const
	{
		return m_value->IsUint();
	}

	template <>
	bool UGenericValue::Get(int32_t& outInt) const
	{
		if (!m_value->IsInt())
		{
			return false;
		}

		outInt = m_value->GetInt();
		return true;
	}

	template <>
	bool UGenericValue::IsA<int32_t>() const
	{
		return m_value->IsInt();
	}

	template <>
	bool UGenericValue::Get(eastl::string& outString) const
	{
		if (!m_value->IsString())
		{
			return false;
		}

		outString = m_value->GetString();
		return true;
	}

	template <>
	bool UGenericValue::IsA<eastl::string>() const
	{
		return m_value->IsString();
	}

	template <>
	bool UGenericValue::Get(bool& outBool) const
	{
		if (!m_value->IsBool())
		{
			return false;
		}

		outBool = m_value->GetBool();
		return true;
	}

	template <>
	bool UGenericValue::IsA<bool>() const
	{
		return m_value->IsBool();
	}

	template <>
	bool UGenericValue::Get<Vector2>(Vector2& outVector2) const
	{
		if (!IsA<Vector2>())
		{
			return false;
		}

		UArrayValue valueArray(m_value);

		valueArray[0].Get(outVector2.x);
		valueArray[1].Get(outVector2.y);

		return true;
	}

	template <>
	bool UGenericValue::Get(Vector3& outVector3) const
	{
		if (!IsA<Vector3>())
		{
			return false;
		}
		
		UArrayValue valueArray(m_value);

		valueArray[0].Get(outVector3.x);
		valueArray[1].Get(outVector3.y);
		valueArray[2].Get(outVector3.z);

		return true;
	}

	template <>
	bool UGenericValue::IsA<Vector2>() const
	{
		if (!m_value->IsArray() || m_value->Size() != 2)
		{
			return false;
		}

		UArrayValue valueArray(m_value);

		for (SizeType i = 0; i < 2; ++i)
		{
			if (!valueArray[i].IsA<double>())
			{
				return false;
			}
		}

		return true;
	}

	template <>
	bool UGenericValue::IsA<Vector3>() const
	{
		if (!m_value->IsArray() || m_value->Size() != 3)
		{
			return false;
		}

		UArrayValue valueArray(m_value);

		for (SizeType i = 0; i < 3; ++i)
		{
			if (!valueArray[i].IsA<double>())
			{
				return false;
			}
		}

		return true;
	}

	template <>
	bool UGenericValue::IsA<Vector4>() const
	{
		if (!m_value->IsArray() || m_value->Size() != 4)
		{
			return false;
		}

		UArrayValue valueArray(m_value);

		for (SizeType i = 0; i < 4; ++i)
		{
			if (!valueArray[i].IsA<double>())
			{
				return false;
			}
		}

		return true;
	}

	template <>
	bool UGenericValue::Get(Color& outColor) const
	{
		if (!IsA<Vector4>())
		{
			return false;
		}

		UArrayValue valueArray(m_value);

		valueArray[0].Get(outColor.x);
		valueArray[1].Get(outColor.y);
		valueArray[2].Get(outColor.z);
		valueArray[3].Get(outColor.w);

		return true;
	}

	template <>
	bool UGenericValue::Get(Quaternion& outQuaternion) const
	{
		if (!m_value->IsArray() || m_value->Size() != 4)
		{
			return false;
		}

		for (SizeType i = 0; i < 3; i++)
		{
			if (!m_value[i].IsDouble())
			{
				return false;
			}
		}

		outQuaternion.x = m_value[0].GetFloat();
		outQuaternion.y = m_value[1].GetFloat();
		outQuaternion.z = m_value[2].GetFloat();
		outQuaternion.w = m_value[3].GetFloat();

		return true;
	}

	template <>
	bool UGenericValue::Get(UObjectValue& outObjectValue) const
	{
		if (!m_value->IsObject())
		{
			return false;
		}

		outObjectValue = UObjectValue(m_value);
		return true;
	}

	template <>
	bool UGenericValue::Get(UArrayValue& outArrayValue) const
	{
		if (!m_value->IsArray())
		{
			return false;
		}

		outArrayValue = UArrayValue(m_value);
		return true;
	}



	template <>
	bool UObjectValue::GetProperty(const char* inPropName, UObjectValue& outObjectValue) const
	{
		auto propFindIter = m_objectValue.m_value->FindMember(inPropName);

		if (propFindIter == m_objectValue.m_value->MemberEnd())
		{
			return false;
		}

		outObjectValue = UObjectValue(&propFindIter->value);
		return true;
	}

	template <>
	bool UObjectValue::GetProperty(const char* inPropName, UArrayValue& outArrayValue) const
	{
		auto propFindIter = m_objectValue.m_value->FindMember(inPropName);

		if (propFindIter == m_objectValue.m_value->MemberEnd())
		{
			return false;
		}

		outArrayValue = UArrayValue(&propFindIter->value);
		return true;
	}

	UGenericValue UArrayValue::operator[](SizeType inIndex) const
	{
		return UGenericValue(&m_arrayElementValue.m_value->GetArray()[inIndex]);
	}

	SizeType UArrayValue::Size() const
	{
		return m_arrayElementValue.m_value->GetArray().Size();
	}


}