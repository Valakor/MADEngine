#include "Core/Component.h"
#include "Core/Entity.h"

namespace MAD
{
	bool UComponent::IsOwnerValid() const
	{
		// A component is valid only if it's owner isn't marked pending for kill
		return !GetOwner().IsPendingForKill();
	}

}
