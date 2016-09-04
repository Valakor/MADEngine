#pragma once

#include "Core/Object.h"

#include <EASTL/weak_ptr.h>

namespace MAD
{
	class AEntity;

	class UComponent : public UObject
	{
		MAD_DECLARE_CLASS(UComponent, UObject)

	public:
		virtual ~UComponent() { }

		virtual void UpdateComponent(float inDeltaTime) { (void)inDeltaTime; };
		
		inline void SetOwner(AEntity& inOwner) { m_ownerWeakPtr = &inOwner; }
		inline AEntity& GetOwner() { return *m_ownerWeakPtr; }
		inline const AEntity& GetOwner() const { return *m_ownerWeakPtr; }

	private:
		AEntity* m_ownerWeakPtr;
	};
}
