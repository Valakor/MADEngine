#include "Core/Component.h"

#include "Core/Entity.h"
#include "Core/GameInput.h"

namespace MAD
{
	UComponent::UComponent(OGameWorld* inOwningWorld)
		: Super(inOwningWorld)
		, m_ownerPtr(nullptr) { }

	bool UComponent::IsOwnerValid() const
	{
		// A component is valid only if it's owner isn't marked pending for kill
		return !GetOwner().IsPendingForKill();
	}
}
