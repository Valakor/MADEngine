#pragma once

#include "ObjectTypeInfo.h"
#include "Networking/Network.h"

namespace MAD
{
	class OGameWorld;

	using ObjectID = uint64_t;
	const ObjectID InvalidObjectID = eastl::numeric_limits<ObjectID>::max();
	
	class UObject
	{
		MAD_DECLARE_BASE_CLASS(UObject)
	public:
		explicit UObject(OGameWorld* inOwningGameWorld);

		virtual ~UObject();

		virtual void GetReplicatedProperties(eastl::vector<SObjectReplInfo>& inOutReplInfo) const { (void)inOutReplInfo; }

		inline ObjectID GetObjectID() const { return m_objectID; }
		inline SNetworkID GetNetID() const { return m_netID; }
		inline OGameWorld* GetOwningWorld() { return m_owningGameWorld; }
		inline const OGameWorld* GetOwningWorld() const { return m_owningGameWorld; }

		void SetNetID(SNetworkID inNetID);
		bool IsNetworkSpawned() const { return m_netID.IsValid(); }

		ENetMode GetNetMode() const;

		bool IsValid() const { return !m_isDestroyed; }
		virtual void Destroy();

	protected:
		virtual void OnDestroy() {}

	private:
		static ObjectID s_objectRunningUID;

		ObjectID m_objectID;
		SNetworkID m_netID;
		OGameWorld* m_owningGameWorld;

		bool m_isDestroyed;
	};
}
