#pragma once

#include "ObjectTypeInfo.h"
#include "Networking/Network.h"

namespace MAD
{
	class OGameWorld;

	using ObjectID_t = uint64_t;

	const ObjectID_t InvalidObjectID = eastl::numeric_limits<ObjectID_t>::max();
	
	class UObject
	{
		MAD_DECLARE_BASE_CLASS(UObject)
	public:
		explicit UObject(OGameWorld* inOwningGameWorld);

		virtual ~UObject();

		virtual void GetReplicatedProperties(eastl::vector<SObjectReplInfo>& inOutReplInfo) const { (void)inOutReplInfo; }
		virtual void OnEvent(EEventTypes inEventType, void* inEventData) { (void)inEventType; (void)inEventData; }

		inline ObjectID_t GetObjectID() const { return m_objectID; }
		inline OGameWorld* GetOwningWorld() { return m_owningGameWorld; }
		inline const OGameWorld* GetOwningWorld() const { return m_owningGameWorld; }

		void SetNetIdentity(SNetworkID inNetID, ENetRole::Type inNetRole, class ONetworkPlayer* inNetOwner);
		bool IsNetworkSpawned() const { return m_netID.IsValid(); }

		ENetMode GetNetMode() const;
		SNetworkID GetNetID() const { return m_netID; }
		ENetRole::Type GetNetRole() const { return m_netRole; }
		class ONetworkPlayer* GetNetOwner() const { return m_netOwner; }

		bool IsValid() const { return !m_isDestroyed; }
		virtual void Destroy();

	protected:
		virtual void OnDestroy() {}

	private:
		static ObjectID_t s_objectRunningUID;

		ObjectID_t m_objectID;
		OGameWorld* m_owningGameWorld;

		SNetworkID m_netID;
		ENetRole::Type m_netRole;
		class ONetworkPlayer* m_netOwner;

		bool m_isDestroyed;
	};
}
