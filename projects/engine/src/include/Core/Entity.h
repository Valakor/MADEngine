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

	using ComponentContainer = eastl::vector<eastl::weak_ptr<UComponent>>;
	using ConstComponentContainer = eastl::vector<eastl::weak_ptr<const UComponent>>;

	class AEntity : public UObject
	{
		MAD_DECLARE_ACTOR(AEntity, UObject)
	public:
		explicit AEntity(OGameWorld* inOwningWorld);
		virtual ~AEntity() {}

		void PostInitialize();
		void BeginPlay();

		virtual void GetReplicatedProperties(eastl::vector<SObjectReplInfo>& inOutReplInfo) const override;

		virtual void OnEvent(EEventTypes inEventType, void* inEventData) override;

		bool AttachEntity(eastl::shared_ptr<AEntity> inChildEntity);

		virtual void Destroy() override;
		bool IsPendingForKill() const { return m_isPendingForKill; }
	
		const ULinearTransform& GetWorldTransform() const;
		float GetWorldScale() const;
		const Quaternion& GetWorldRotation() const;
		const Vector3& GetWorldTranslation() const;

		void SetWorldScale(float inScale);
		void SetWorldRotation(const Quaternion& inRotation);
		void SetWorldTranslation(const Vector3& inTranslation);

		AEntity* GetParent() const;
		UComponent* GetRootComponent() const { return m_rootComponent.get(); }
		void GetEntityComponents(ConstComponentContainer& inOutConstEntityComponents) const;
		void GetEntityComponents(ComponentContainer& inOutEntityComponents);
		eastl::weak_ptr<const UComponent> GetEntityComponentByIndex(size_t inIndex) const { return m_entityComponents[inIndex]; }
		eastl::weak_ptr<UComponent> GetEntityComponentByIndex(size_t inIndex) { return m_entityComponents[inIndex]; }

		size_t GetComponentCount() const { return m_entityComponents.size(); }

		// Gets the first component of the input type. Returns weak_ptr because external users shouldn't maintain strong references
		// to an entity's components
		template <typename ComponentType>
		eastl::weak_ptr<const ComponentType> GetFirstComponentByType() const;
		template <typename ComponentTypeBase>
		eastl::weak_ptr<const ComponentTypeBase> GetFirstComponentByType(const TTypeInfo& inTypeInfo) const;

		template <typename ComponentType>
		eastl::weak_ptr<ComponentType> GetFirstComponentByType();
		template <typename ComponentTypeBase>
		eastl::weak_ptr<ComponentTypeBase> GetFirstComponentByType(const TTypeInfo& inTypeInfo);

		template <typename ComponentType>
		eastl::vector<eastl::weak_ptr<ComponentType>> GetComponentsByType();

		OGameWorld& GetWorld();
		const OGameWorld& GetWorld() const;
		OGameWorldLayer& GetOwningWorldLayer() { return *m_owningWorldLayer; }
		const OGameWorldLayer& GetOwningWorldLayer() const { return *m_owningWorldLayer; }
		void SetOwningWorldLayer(OGameWorldLayer& inWorldLayer) { m_owningWorldLayer = &inWorldLayer; }

		void PrintTranslationHierarchy() const;
	protected:
		virtual void PostInitializeComponents() {}
		virtual void OnBeginPlay() {}

		// Component creation API
		// WARNING: Currently, entities should only add components to themselves within their constructors because they're only registered to the component updater
		// as a progress of the entity construction process. If you try to add components outside of the constructor, they will not update.
		template <typename ComponentType>
		eastl::shared_ptr<ComponentType> AddComponent();

		template <typename ComponentType>
		eastl::shared_ptr<ComponentType> AddComponent(const TTypeInfo& inTypeInfo);

		void SetRootComponent(eastl::shared_ptr<UComponent> inEntityRootComp);
	private:
		void AttachChild(eastl::shared_ptr<AEntity> inChildEntity);

		void DetachFromParent();
	private:
		friend class UGameWorldLoader;

		bool m_isPendingForKill;

		AEntity* m_owningEntity;
		OGameWorldLayer* m_owningWorldLayer;

		eastl::shared_ptr<UComponent> m_rootComponent;

		eastl::vector<eastl::shared_ptr<UComponent>> m_entityComponents;
		eastl::vector<eastl::weak_ptr<AEntity>> m_entityChildren;
	};

	// WARNING: Since we are giving shared_ptrs of the attached components, users that reference attached components must be careful of it's owner entity being destroyed.
	// You can check if the component's owner is still valid by calling IsOwnerValid, which just checks if it's owner is not pending for kill.

	template <typename ComponentType>
	eastl::shared_ptr<ComponentType> AEntity::AddComponent()
	{
		static_assert(eastl::is_base_of<UComponent, ComponentType>::value, "Error: You may only create components that are of type UComponent or more derived");
		return AddComponent<ComponentType>(*ComponentType::StaticClass());
	}

	template <typename ComponentType>
	eastl::shared_ptr<ComponentType> AEntity::AddComponent(const TTypeInfo& inTypeInfo)
	{
		static_assert(eastl::is_base_of<UComponent, ComponentType>::value, "Error: You may only create components that are of type UComponent or more derived");

		eastl::shared_ptr<ComponentType> newComponent = CreateDefaultObject<ComponentType>(inTypeInfo, GetOwningWorld());;

		newComponent->SetOwningEntity(*this);
		
		MAD_ASSERT_DESC(GetOwningWorld() != nullptr, "Error: Every entity should have a valid owning UGameWorld!!!");

		if (!GetOwningWorld())
		{
			return nullptr;
		}

		// Before adding the component to the entity, we need to register it with the owning world's component updater
		GetOwningWorld()->GetComponentUpdater().RegisterComponent(newComponent);
		
		m_entityComponents.push_back(newComponent);

		return newComponent;
	}

	template <typename ComponentType>
	eastl::weak_ptr<const ComponentType> AEntity::GetFirstComponentByType() const
	{
		for (const auto& currentComponent : m_entityComponents)
		{
			if (IsA<ComponentType>(currentComponent.get()))
			{
				return eastl::reinterpret_pointer_cast<const ComponentType>(currentComponent);
			}
		}

		return eastl::weak_ptr<const ComponentType>();
	}

	template <typename ComponentTypeBase>
	eastl::weak_ptr<const ComponentTypeBase> AEntity::GetFirstComponentByType(const TTypeInfo& inTypeInfo) const
	{
		static_assert(eastl::is_base_of<UComponent, ComponentTypeBase>::value, "Error: Components must be of type UComponent or more derived");

		for (const auto& currentComponent : m_entityComponents)
		{
			if (IsA(inTypeInfo, currentComponent.get()))
			{
				return eastl::reinterpret_pointer_cast<const ComponentTypeBase>(currentComponent);
			}
		}

		return eastl::weak_ptr<const ComponentTypeBase>();
	}

	template <typename ComponentType>
	eastl::weak_ptr<ComponentType> AEntity::GetFirstComponentByType()
	{
		for (const auto& currentComponent : m_entityComponents)
		{
			if (IsA<ComponentType>(currentComponent.get()))
			{
				return eastl::reinterpret_pointer_cast<ComponentType>(currentComponent);
			}
		}

		return eastl::weak_ptr<ComponentType>();
	}

	template <typename ComponentTypeBase>
	eastl::weak_ptr<ComponentTypeBase> AEntity::GetFirstComponentByType(const TTypeInfo& inTypeInfo)
	{
		static_assert(eastl::is_base_of<UComponent, ComponentTypeBase>::value, "Error: Components must be of type UComponent or more derived");

		for (const auto& currentComponent : m_entityComponents)
		{
			if (IsA(inTypeInfo, currentComponent.get()))
			{
				return eastl::reinterpret_pointer_cast<ComponentTypeBase>(currentComponent);
			}
		}

		return eastl::weak_ptr<ComponentTypeBase>();
	}

	template <typename ComponentType>
	eastl::vector<eastl::weak_ptr<ComponentType>> AEntity::GetComponentsByType()
	{
		eastl::vector<eastl::weak_ptr<ComponentType>> foundComps;

		for (const auto& currentComponent : m_entityComponents)
		{
			if (IsA<ComponentType>(currentComponent.get()))
			{
				foundComps.push_back(eastl::reinterpret_pointer_cast<ComponentType>(currentComponent));
			}
		}

		return foundComps;
	}
}
