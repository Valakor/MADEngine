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
		// Use parent component's world transform and find the difference in scale needed to produce the world scale desired
		float adjustedLocalScale = inScale;

		if (m_parentComponent)
		{
			// childLScale * parentWScale = inScale
			// childLScale = inScale / parentWScale
			// childLScale is the relative scale from it's parent that the child needs to achieve a world scale of inScale
			adjustedLocalScale = inScale / m_parentComponent->m_componentWorldTransform.GetScale();
		}

		m_componentLocalTransform.SetScale(adjustedLocalScale);
		
		UpdateWorldTransform();
	}

	void UComponent::SetWorldRotation(const Quaternion& inRotation)
	{
		Quaternion adjustedLocalRotation = inRotation;

		if (m_parentComponent)
		{
			// childLR * parentWR = inRotation
			// childLR = inRotation * parentWRInverse
			// childLR is the relative rotation from it's parent that the child needs to achieve a world rotation of inRotation

			Quaternion parentWorldRotationInverse;
			
			m_parentComponent->m_componentWorldTransform.GetRotation().Inverse(parentWorldRotationInverse);

			adjustedLocalRotation = Quaternion::Concatenate(inRotation, parentWorldRotationInverse);
		}

		m_componentLocalTransform.SetRotation(adjustedLocalRotation);

		UpdateWorldTransform();
	}

	void UComponent::SetWorldTranslation(const Vector3& inTranslation)
	{
		Vector3 adjustedLocalTranslation = inTranslation;

		if (m_parentComponent)
		{
			// childLTranslation + parentWTranslation = inTranslation
			// childLTranslation = inTranslation - parentWTranslation
			// childLTranslation is the relative translation from it's parent that the child needs to achieve a world translation of inTranslation

			adjustedLocalTranslation = inTranslation - m_parentComponent->m_componentWorldTransform.GetTranslation();
		}

		m_componentLocalTransform.SetTranslation(adjustedLocalTranslation);

		UpdateWorldTransform();
	}

	bool UComponent::IsOwnerValid() const
	{
		// A component is valid only if it's owner isn't marked pending for kill
		return !GetOwningEntity().IsPendingForKill();
	}

	void UComponent::UpdateWorldTransform()
	{
		// If a component doesn't have a parent, it's local transform is equal to it's world transform
		if (m_parentComponent)
		{
			m_componentWorldTransform = m_componentLocalTransform * m_parentComponent->m_componentWorldTransform;
		}
		else
		{
			m_componentWorldTransform = m_componentLocalTransform;
		}

		UpdateChildWorldTransforms();
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
		if (inParent)
		{
			m_parentComponent = inParent->GetRootComponent();

			// Add this component to the root component's children list so that we can receive spatial transform updates
		}
	}

	void UComponent::DetachFromParent()
	{
		// Remove myself from my parent's child list
		if (m_parentComponent)
		{
			
		}
	}

}
