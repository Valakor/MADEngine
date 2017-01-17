#include "Core/Entity.h"
#include "Core/GameWorld.h"
#include "Core/GameWorldLayer.h"

namespace MAD
{
	AEntity::AEntity(OGameWorld* inOwningWorld)
		: Super_t(inOwningWorld)
		, m_isPendingForKill(false)
		, m_owningEntity(nullptr)
		, m_owningWorldLayer(nullptr)
		, m_rootComponent(nullptr) {}

	void AEntity::PostInitialize()
	{
		// If an entity doesn't have any children component, attach a default scene component (?)
		if (m_entityComponents.empty())
		{
			AddComponent<UComponent>();
		}

		// Cases we have to worry about:
		// 1. An entity that initializes its own components in constructor and assigns root component properly before attaching other components
		// 2. An entity loaded (root component only attached at this point)
		// 3. An entity that initializes its own components in constructor and doesn't assign root component (root component only attached at this point)

		if (!m_entityComponents.empty() && m_rootComponent == nullptr)
		{
			// If an entity doesn't have a root, find the first component tree root and assign that to be the root component
			for (const auto& currentChildComp : m_entityComponents)
			{
				if (!currentChildComp->GetParent())
				{
					m_rootComponent = currentChildComp;
					break;
				}
			}

			MAD_ASSERT_DESC(m_rootComponent != nullptr, "Error: An entity shouldn't have no components that are roots of a component tree");
		}

		m_rootComponent->UpdateWorldTransform();

		for (auto& currentChildComp : m_entityComponents)
		{
			// We only want to attach components without parents to the root component (indication of tree root)
			if (currentChildComp != m_rootComponent && !currentChildComp->GetParent())
			{
				m_rootComponent->AttachComponent(currentChildComp);
			}
		}

		if (IsNetworkSpawned())
		{
			for (auto component : m_entityComponents)
			{
				component->SetNetIdentity(GetNetID(), GetNetRole(), GetNetOwner());
			}
		}

		// At this point, the components' hierarchy is setup properly and world transforms are computed properly. Components can
		// assume that they have a root component (except the root component)

		for (auto component : m_entityComponents)
		{
			component->PostInitializeComponents();
		}

		PostInitializeComponents();
	}

	void AEntity::BeginPlay()
	{
		for (auto component : m_entityComponents)
		{
			component->OnBeginPlay();
		}

		OnBeginPlay();
	}

	void AEntity::Destroy()
	{
		// Destroying an Entity means to basically destroy all of it's attached components
		// HOWEVER, an Entity shouldn't be destroyed right away or else we will run into issues if
		// we try to destroy an Entity within a Component's update function. Destroying an Entity would mean that we need to destroy all
		// of it's components, but obviously we can't modify the component lists while iterating over them'
		// THEREFORE, we must mark the entity as pending for kill so that we can safely destroy the entity
		m_isPendingForKill = true;

		// If our root component has a parent (i.e this entity is parented to another entity), we need to remove ourselves from their root component's
		// child list
		DetachFromParent();

		Super_t::Destroy();
	}

	Vector3 AEntity::GetForward() const
	{
		MAD_ASSERT_DESC(m_rootComponent != nullptr, "Error: Cannot get forward vector for an entity that doesn't have a root component");

		return m_rootComponent->GetComponentForward();
	}

	Vector3 AEntity::GetRight() const
	{
		MAD_ASSERT_DESC(m_rootComponent != nullptr, "Error: Cannot get right vector for an entity that doesn't have a root component");

		return m_rootComponent->GetComponentRight();
	}

	Vector3 AEntity::GetUp() const
	{
		MAD_ASSERT_DESC(m_rootComponent != nullptr, "Error: Cannot get up vector for an entity that doesn't have a root component");

		return m_rootComponent->GetComponentUp();
	}

	OGameWorld& AEntity::GetWorld()
	{
		return *m_owningWorldLayer->GetOwningWorld();
	}

	const OGameWorld& AEntity::GetWorld() const
	{
		return *m_owningWorldLayer->GetOwningWorld();
	}

	void AEntity::PrintTranslationHierarchy() const
	{
		MAD_ASSERT_DESC(m_rootComponent != nullptr, "Error: Cannot print translation hierarchy for an entity that doesn't have a root component");

		return m_rootComponent->PrintTranslationHierarchy(0);
	}

	void AEntity::PopulateTransformQueue(eastl::queue<ULinearTransform>& inOutTransformQueue) const
	{
		if (!m_rootComponent)
		{
			return;
		}

		m_rootComponent->PopulateTransformQueue(inOutTransformQueue);
	}

	void AEntity::GetReplicatedProperties(eastl::vector<SObjectReplInfo>& inOutReplInfo) const
	{
		Super_t::GetReplicatedProperties(inOutReplInfo);

		// Replicate any Entity properties (probably none by default?)

		// Replicate all component properties
		for (const auto component : m_entityComponents)
		{
			component->GetReplicatedProperties(inOutReplInfo);
		}
	}

	void AEntity::OnEvent(EEventTypes inEventType, void* inEventData)
	{
		Super_t::OnEvent(inEventType, inEventData);

		for (auto& currentChildComp : m_entityComponents)
		{
			currentChildComp->OnEvent(inEventType, inEventData);
		}
	}

	bool AEntity::AttachEntity(eastl::shared_ptr<AEntity> inChildEntity)
	{
		if (m_rootComponent && inChildEntity && inChildEntity->m_rootComponent)
		{
			AttachChild(inChildEntity);

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
		// An entity can only have a parent entity if it's root component has a parent component
		if (m_rootComponent && m_rootComponent->GetParent())
		{
			return &m_rootComponent->GetParent()->GetOwningEntity();
		}

		return nullptr;
	}

	void AEntity::GetEntityComponents(ConstComponentContainer_t& inOutConstEntityComponents) const
	{
		for (auto& currentComponent : m_entityComponents)
		{
			inOutConstEntityComponents.emplace_back(currentComponent);
		}
	}

	void AEntity::GetEntityComponents(ComponentContainer_t& inOutEntityComponents)
	{
		for (auto& currentComponent : m_entityComponents)
		{
			inOutEntityComponents.emplace_back(currentComponent);
		}
	}

	void AEntity::SetRootComponent(eastl::shared_ptr<UComponent> inEntityRootComp)
	{
		// We shouldn't be allowed to set a component as the root component if it has a parent component already (?)
		if (inEntityRootComp && !inEntityRootComp->GetParent())
		{
			// Validate that this component is actually a component of the entity
			for (const auto& currentEntityComp : m_entityComponents)
			{
				if (currentEntityComp == inEntityRootComp)
				{
					m_rootComponent = currentEntityComp;
					return;
				}
			}
		}
	}

	void AEntity::AttachChild(eastl::shared_ptr<AEntity> inChildEntity)
	{
		m_rootComponent->AttachComponent(inChildEntity->m_rootComponent);

		m_entityChildren.push_back(inChildEntity);
	}

	void AEntity::DetachFromParent()
	{
		if (m_rootComponent && m_rootComponent->m_parentComponent)
		{
			// Remove myself from my parent's child list
			UComponent* parentEntityRoot = m_rootComponent->m_parentComponent;
			
			auto childCompFindIter = eastl::find(parentEntityRoot->m_childComponents.cbegin(), parentEntityRoot->m_childComponents.cend(), m_rootComponent);

			if (childCompFindIter != parentEntityRoot->m_childComponents.cend())
			{
				parentEntityRoot->m_childComponents.erase(childCompFindIter);
			}
		}
	}
}
