#include "Core/Entity.h"
#include "Core/GameWorld.h"
#include "Core/GameWorldLayer.h"

namespace MAD
{
	AEntity::AEntity(OGameWorld* inOwningWorld)
		: Super(inOwningWorld)
		, m_isPendingForKill(false)
		, m_owningWorldLayer(nullptr)
		, m_owningEntity(nullptr)
	{}

	void AEntity::Destroy()
	{
		// Destroying an Entity means to basically destroy all of it's attached components
		// HOWEVER, an Entity shouldn't be destroyed right away or else we will run into issues if
		// we try to destroy an Entity within a Component's update function. Destroying an Entity would mean that we need to destroy all
		// of it's components, but obviously we can't modify the component lists while iterating over them'
		// THEREFORE, we must mark the entity as pending for kill so that we can safely destroy the entity
		m_isPendingForKill = true;
	}

	OGameWorld& AEntity::GetWorld()
	{
		return *m_owningWorldLayer->GetOwningWorld();
	}

	const OGameWorld& AEntity::GetWorld() const
	{
		return *m_owningWorldLayer->GetOwningWorld();
	}

	// Attempts at attaching this entity to another entity as a parent. Returns true on success and false on failure
	bool AEntity::AttachTo(AEntity* inParentEntity)
	{
		if (m_rootComponent && inParentEntity)
		{
			m_rootComponent->AttachToParent(inParentEntity);

			return true;
		}

		return false;
	}

	const ULinearTransform& AEntity::GetWorldTransform() const
	{
		MAD_ASSERT_DESC(m_rootComponent != nullptr, "Error: Cannot retrieve the world transform for an entity that doesn't have a root component");

		return m_rootComponent->GetWorldTransform();
	}

	float AEntity::GetWorldScale() const
	{
		MAD_ASSERT_DESC(m_rootComponent != nullptr, "Error: Cannot retrieve the world scale of an entity that doesn't have a root component");

		return m_rootComponent->GetWorldScale();
	}

	const Quaternion& AEntity::GetWorldRotation() const
	{
		MAD_ASSERT_DESC(m_rootComponent != nullptr, "Error: Cannot retrieve the world rotation of an entity that doesn't have a root component");

		return m_rootComponent->GetWorldRotation();
	}

	const Vector3& AEntity::GetWorldTranslation() const
	{
		MAD_ASSERT_DESC(m_rootComponent != nullptr, "Error: Cannot retrieve the world translation of an entity that doesn't have a root component");

		return m_rootComponent->GetWorldTranslation();
	}

	void AEntity::SetWorldScale(float inScale)
	{
		MAD_ASSERT_DESC(m_rootComponent != nullptr, "Error: Cannot set the world scale of an entity that doesn't have a root component");

		return m_rootComponent->SetWorldScale(inScale);
	}

	void AEntity::SetWorldRotation(const Quaternion& inRotation)
	{
		MAD_ASSERT_DESC(m_rootComponent != nullptr, "Error: Cannot set the world rotation of an entity that doesn't have a root component");

		return m_rootComponent->SetWorldRotation(inRotation);
	}

	void AEntity::SetWorldTranslation(const Vector3& inTranslation)
	{
		MAD_ASSERT_DESC(m_rootComponent != nullptr, "Error: Cannot set the world translation of an entity that doesn't have a root component");

		return m_rootComponent->SetWorldTranslation(inTranslation);
	}

	AEntity* AEntity::GetParent() const
	{
		if (m_rootComponent && m_rootComponent->GetParent())
		{
			return &m_rootComponent->GetParent()->GetOwningEntity();
		}

		return nullptr;
	}

	void AEntity::GetEntityComponents(ConstComponentContainer& inOutConstEntityComponents) const
	{
		for (auto& currentComponent : m_entityComponents)
		{
			inOutConstEntityComponents.emplace_back(currentComponent);
		}
	}

	void AEntity::GetEntityComponents(ComponentContainer& inOutEntityComponents)
	{
		for (auto& currentComponent : m_entityComponents)
		{
			inOutEntityComponents.emplace_back(currentComponent);
		}
	}

	void AEntity::SetRootComponent(UComponent* inEntityRootComp)
	{
		// We shouldn't be allowed to set a component as the root component if it has a parent component already (?)
		if (inEntityRootComp && !inEntityRootComp->GetParent())
		{
			m_rootComponent = inEntityRootComp;
		}
	}
}
