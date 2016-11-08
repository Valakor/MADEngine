#include "Core/Component.h"

#include "Core/Entity.h"
#include "Core/GameInput.h"

namespace MAD
{
	DECLARE_LOG_CATEGORY(LogBaseComponent);

	UComponent::UComponent(OGameWorld* inOwningWorld)
		: Super(inOwningWorld)
		, m_ownerPtr(nullptr) { }
		, m_parentComponent(nullptr)
	{}

	void UComponent::AttachComponent(eastl::shared_ptr<UComponent> inChildComponent)
	{
		if (inChildComponent)
		{
			m_childComponents.push_back(inChildComponent);
			inChildComponent->m_parentComponent = this;

			inChildComponent->UpdateWorldTransform();
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

		SetRelativeScale(adjustedLocalScale);
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

		SetRelativeRotation(adjustedLocalRotation);
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

		SetRelativeTranslation(adjustedLocalTranslation);
	}

	void UComponent::SetRelativeScale(float inScale)
	{
		m_componentLocalTransform.SetScale(inScale);

		UpdateWorldTransform();
	}

	void UComponent::SetRelativeRotation(const Quaternion& inRotation)
	{
		m_componentLocalTransform.SetRotation(inRotation);

		UpdateWorldTransform();
	}

	void UComponent::SetRelativeTranslation(const Vector3& inTranslation)
	{
		m_componentLocalTransform.SetTranslation(inTranslation);

		UpdateWorldTransform();
	}

	bool UComponent::IsOwnerValid() const
	{
		// A component is valid only if it's owner isn't marked pending for kill
		return !GetOwningEntity().IsPendingForKill();
	}

	void UComponent::PrintTranslationHierarchy(uint8_t inDepth) const
	{
		const Vector3 currentWorldTranslation = m_componentWorldTransform.GetTranslation();

		for (size_t i = 0; i < inDepth; ++i)
		{
			OutputDebugString(L">> ");
		}

		LOG(LogBaseComponent, Log, "World Translation: %f, %f, %f\n", currentWorldTranslation.x, currentWorldTranslation.y, currentWorldTranslation.z);

		for (const auto& currentChild : m_childComponents)
		{
			currentChild->PrintTranslationHierarchy(inDepth + 1);
		}
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
}
