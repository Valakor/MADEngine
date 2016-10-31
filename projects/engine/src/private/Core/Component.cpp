#include "Core/Component.h"
#include "Core/Entity.h"

namespace MAD
{

	UComponent::UComponent(OGameWorld* inOwningWorld)
		: Super(inOwningWorld) {}

	bool UComponent::IsOwnerValid() const
	{
		// A component is valid only if it's owner isn't marked pending for kill
		return !GetOwner().IsPendingForKill();
	}

	void UComponent::SetScale(float inScale)
	{
		m_componentScale = inScale;

		UpdateComponentTransform();
	}

	void UComponent::SetRotation(const Quaternion& inRotation)
	{
		
	}

	void UComponent::SetTranslation(const Vector3& inTranslation)
	{
		
	}

	void UComponent::UpdateComponentTransform()
	{
		
	}

}
