#pragma once

#include <EASTL/shared_ptr.h>
#include <EASTL/vector.h>
#include <EASTL/type_traits.h>

#include "Core/Component.h"
#include "Core/Object.h"
#include "Core/GameWorld.h"
#include "Core/SimpleMath.h"
#include "Misc/Assert.h"

namespace MAD
{
	class OGameWorld;
	class OGameWorldLayer;

	class AEntity : public UObject
	{
		MAD_DECLARE_ACTOR(AEntity, UObject)
	public:
		using ComponentContainer = eastl::vector<eastl::weak_ptr<UComponent>>;
		using ConstComponentContainer = eastl::vector<eastl::weak_ptr<const UComponent>>;
	public:
		explicit AEntity(OGameWorld* inOwningWorld);

		virtual void OnBeginPlay() {}
		virtual void PostInitializeComponents() {}

		void Destroy();

		// Utility getter and setter functions
		bool IsPendingForKill() const { return m_isPendingForKill; }

		OGameWorld& GetWorld();
		const OGameWorld& GetWorld() const;

		const ULinearTransform& GetWorldTransform() const { return m_rootComponent->GetWorldTransform(); }
		void SetScale(float inScale);
		void SetRotation(const Quaternion& inRotation);
		void SetPosition(const Vector3& inPosition);

		void GetEntityComponents(ConstComponentContainer& inOutConstEntityComponents) const;
		void GetEntityComponents(ComponentContainer& inOutEntityComponents);
		size_t GetComponentCount() const { return m_actorComponents.size(); }
		OGameWorldLayer& GetOwningWorldLayer() { return *m_owningWorldLayer; }
		const OGameWorldLayer& GetOwningWorldLayer() const { return *m_owningWorldLayer; }
		
		// Gets the first component of the input type. Returns weak_ptr because external users shouldn't maintain strong references
		// to an entity's components
		template <typename ComponentType>
		eastl::weak_ptr<const ComponentType> GetFirstComponentByType() const;

		template <typename ComponentType>
		eastl::weak_ptr<ComponentType> GetFirstComponentByType();

		template <typename ComponentType>
		eastl::vector<eastl::weak_ptr<ComponentType>> GetComponentsByType();

		void SetOwningWorldLayer(OGameWorldLayer& inWorldLayer) { m_owningWorldLayer = &inWorldLayer; }
	protected:
		// Component creation API
		// WARNING: Currently, entities should only add components to themselves within their constructors because they're only registered to the component updater
		// as a progress of the entity construction process. If you try to add components outside of the constructor, they will not update.
		template <typename ComponentType>
		eastl::weak_ptr<ComponentType> AddComponent();

		template <typename ComponentType>
		eastl::weak_ptr<ComponentType> AddComponent(const TTypeInfo& inTypeInfo);
		
		void UpdateEntityWorldTransform();
	private:
		bool m_isPendingForKill;
		OGameWorldLayer* m_owningWorldLayer;

		AEntity* m_entityParent;
		UComponent* m_rootComponent;

		eastl::vector<eastl::shared_ptr<UComponent>> m_actorComponents;
		eastl::vector<eastl::shared_ptr<AEntity>> m_entityChildren;
	};

	// WARNING: Since we are giving shared_ptrs of the attached components, users that reference attached components must be careful of it's owner entity being destroyed.
	// You can check if the component's owner is still valid by calling IsOwnerValid, which just checks if it's owner is not pending for kill.

	template <typename ComponentType>
	eastl::weak_ptr<ComponentType> AEntity::AddComponent()
	{
		static_assert(eastl::is_base_of<UComponent, ComponentType>::value, "Error: You may only create components that are of type UComponent or more derived");
		return AddComponent<ComponentType>(*ComponentType::StaticClass());
	}

	template <typename ComponentType>
	eastl::weak_ptr<ComponentType> AEntity::AddComponent(const TTypeInfo& inTypeInfo)
	{
		static_assert(eastl::is_base_of<UComponent, ComponentType>::value, "Error: You may only create components that are of type UComponent or more derived");

		eastl::shared_ptr<ComponentType> newComponent = inTypeInfo.CreateDefaultObject<ComponentType>(GetOwningWorld());

		newComponent->SetOwner(*this);
		
		MAD_ASSERT_DESC(GetOwningWorld() != nullptr, "Error: Every entity should have a valid owning UGameWorld!!!");

		if (!GetOwningWorld())
		{
			return eastl::weak_ptr<ComponentType>();
		}

		// Before adding the component to the entity, we need to register it with the owning world's component updater
		GetOwningWorld()->GetComponentUpdater().RegisterComponent(newComponent);
		
		m_actorComponents.push_back(newComponent);

		return newComponent;
	}

	template <typename ComponentType>
	eastl::weak_ptr<const ComponentType> AEntity::GetFirstComponentByType() const
	{
		for (const auto& currentComponent : m_actorComponents)
		{
			if (IsA<ComponentType>(currentComponent.get()))
			{
				return eastl::reinterpret_pointer_cast<const ComponentType>(currentComponent);
			}
		}

		return eastl::weak_ptr<const ComponentType>();
	}

	template <typename ComponentType>
	eastl::weak_ptr<ComponentType> AEntity::GetFirstComponentByType()
	{
		for (const auto& currentComponent : m_actorComponents)
		{
			if (IsA<ComponentType>(currentComponent.get()))
			{
				return eastl::reinterpret_pointer_cast<ComponentType>(currentComponent);
			}
		}

		return eastl::weak_ptr<ComponentType>();
	}

	template <typename ComponentType>
	eastl::vector<eastl::weak_ptr<ComponentType>> AEntity::GetComponentsByType()
	{
		eastl::vector<eastl::weak_ptr<ComponentType>> foundComps;

		for (const auto& currentComponent : m_actorComponents)
		{
			if (IsA<ComponentType>(currentComponent.get()))
			{
				foundComps.push_back(eastl::reinterpret_pointer_cast<ComponentType>(currentComponent));
			}
		}

		return foundComps;
	}
}
