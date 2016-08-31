#pragma once

#include "ObjectTypeInfo.h"

namespace MAD
{
	using ObjectID = uint64_t;

	class UObject
	{
		MAD_DECLARE_CLASS(UObject)

	public:
		UObject();

		inline ObjectID GetObjectID() const { return m_objectID; }

	private:
		static ObjectID s_objectRunningUID;

		ObjectID m_objectID;
	};
}
