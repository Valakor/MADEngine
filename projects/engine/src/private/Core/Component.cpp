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
			m_childComponents.push_back(inChildComponent);
			inChildComponent->m_parentComponent = this;
		}
	}

	void UComponent::SetWorldScale(float inScale)
	{
		m_componentWorldTransform.SetScale(inScale);

		UpdateWorldTransform();
	}

	void UComponent::SetWorldRotation(const Quaternion& inRotation)
	{
		m_componentWorldTransform.SetRotation(inRotation);

		UpdateWorldTransform();
	}

	void UComponent::SetWorldTranslation(const Vector3& inTranslation)
	{
		m_componentWorldTransform.SetTranslation(inTranslation);

		UpdateWorldTransform();
	}

	bool UComponent::IsOwnerValid() const
	{
		// A component is valid only if it's owner isn't marked pending for kill
		return !GetOwningEntity().IsPendingForKill();
	}

	void UComponent::UpdateWorldTransform()
	{
		if (m_parentComponent)
		{
			m_componentWorldTransform *= m_parentComponent->m_componentWorldTransform;
		
			UpdateChildWorldTransforms();
		}
	}

	void UComponent::UpdateChildWorldTransforms()
	{
		for (auto& currentChildComp : m_childComponents)
		{
			currentChildComp->UpdateWorldTransform();
		}
	}

	void UComponent::AttachToParent(AEntity* inParent)
	{
		// TODO: Being able to detach and re-attach to another parent?
		if (inParent)
		{
			m_parentComponent = inParent->GetRootComponent();
		}
	}

}
