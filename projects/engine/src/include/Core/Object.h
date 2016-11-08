#pragma once

#include "ObjectTypeInfo.h"
#include "Networking/Networking.h"

namespace MAD
{
	class OGameWorld;

	using ObjectID = uint64_t;
	
	class UObject
	{
		MAD_DECLARE_BASE_CLASS(UObject)
	public:
		explicit UObject(OGameWorld* inOwningGameWorld);

		virtual ~UObject();

		inline ObjectID GetObjectID() const { return m_objectID; }
		inline SNetworkID GetNetID() const { return m_netID; }
		inline OGameWorld* GetOwningWorld() { return m_owningGameWorld; }
		inline const OGameWorld* GetOwningWorld() const { return m_owningGameWorld; }
	private:
		static ObjectID s_objectRunningUID;

		ObjectID m_objectID;
		SNetworkID m_netID;
		OGameWorld* m_owningGameWorld;
	};
}
