#include "Core/Component.h"
#include "Core/Entity.h"

namespace MAD
{

	UComponent::UComponent(OGameWorld* inOwningWorld)
		: Super(inOwningWorld)
	{}

	void UComponent::AttachComponent(eastl::shared_ptr<UComponent> inChildComponent)
	{
		if (inChildComponent)
		{
			m_attachedChildren.push_back(inChildComponent);
			inChildComponent->m_parentComponent = this;
		}
	}

	bool UComponent::IsOwnerValid() const
	{
		// A component is valid only if it's owner isn't marked pending for kill
		return !GetOwningEntity().IsPendingForKill();
	}
}
