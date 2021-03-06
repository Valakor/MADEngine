#include "Core/Component.h"

#include "Core/Entity.h"
#include "Core/GameInput.h"

namespace MAD
{
	DECLARE_LOG_CATEGORY(LogBaseComponent);

	UComponent::UComponent(OGameWorld* inOwningWorld)
		: Super_t(inOwningWorld)
		, m_owningEntity(nullptr)
		, m_isActive(true)
		, m_parentComponent(nullptr) {}

	void UComponent::Destroy()
	{
		Super_t::Destroy();

		m_isActive = false;
	}

	void UComponent::AttachComponent(eastl::shared_ptr<UComponent> inChildComponent)
	{
		if (inChildComponent)
		{
			auto targetCompFindIter = eastl::find(m_childComponents.cbegin(), m_childComponents.cend(), inChildComponent);

			MAD_ASSERT_DESC(targetCompFindIter == m_childComponents.cend(), "Error: Trying to attach a child component that is already a child component");

			if (targetCompFindIter != m_childComponents.cend())
			{
				return;
			}

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
			// Find the delta between the target position and the current position and apply that to our local position to move ourselves
			//adjustedLocalTranslation += (inTranslation - m_componentWorldTransform.GetTranslation());
			adjustedLocalTranslation = inTranslation - m_parentComponent->GetWorldTransform().GetTranslation();
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

	Vector3 UComponent::GetComponentForward() const
	{
		return m_componentWorldTransform.GetForward();
	}

	Vector3 UComponent::GetComponentRight() const
	{
		return m_componentWorldTransform.GetRight();
	}

	Vector3 UComponent::GetComponentUp() const
	{
		return m_componentWorldTransform.GetUp();
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

	void UComponent::PopulateTransformQueue(eastl::queue<ULinearTransform>& inOutTransformQueue) const
	{
		// Push it's own world transform on to the stack and then passes it to it's children in order
		inOutTransformQueue.push(m_componentWorldTransform);

		for (const auto& currentChildComp : m_childComponents)
		{
			currentChildComp->PopulateTransformQueue(inOutTransformQueue);
		}
	}

	void UComponent::UpdateWorldTransform()
	{
		// If a component doesn't have a parent, it's local transform is equal to it's world transform
		if (m_parentComponent)
		{
			m_componentWorldTransform = ULinearTransform::TransformRelative(m_componentLocalTransform, m_parentComponent->m_componentWorldTransform);
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
