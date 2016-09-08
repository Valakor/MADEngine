#pragma once

#include "Core/Object.h"

namespace MAD
{
	class AEntity;

	class UComponent : public UObject
	{
		MAD_DECLARE_BASE_COMPONENT(UComponent, UObject)
	public:
		explicit UComponent(OGameWorld* inOwningWorld);

		virtual ~UComponent() { }

		virtual void UpdateComponent(float inDeltaTime) { (void)inDeltaTime; };

		bool IsOwnerValid() const;
		inline void SetOwner(AEntity& inOwner) { m_ownerPtr = &inOwner; }
		inline AEntity& GetOwner() { return *m_ownerPtr; }
		inline const AEntity& GetOwner() const { return *m_ownerPtr; }
	private:
		AEntity* m_ownerPtr;
	};
}
