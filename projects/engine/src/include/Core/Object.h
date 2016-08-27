#pragma once

#include "ObjectTypeInfo.h"

#include <cstdint>
#include <type_traits>

namespace MAD
{
	using ObjectID = uint64_t;

	class UObject
	{
		MAD_DECLARE_CLASS(UObject)
	private:
		static uint64_t s_objectRunningUID;
	public:
		UObject();

		inline ObjectID GetObjectID() const { return m_objectID; }
	private:
		ObjectID m_objectID;
	};
}
