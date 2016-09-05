#pragma once

#include <EASTL/shared_ptr.h>
#include <EASTL/vector.h>
#include <EASTL/type_traits.h>

#include "Core/Component.h"
#include "Core/Object.h"
#include "Core/GameWorld.h"

namespace MAD
{
	class UGameWorld;
	class UGameWorldLayer;

	class AEntity : public UObject
	{
		MAD_DECLARE_ACTOR(AEntity, UObject)
	public:
		AEntity();

		virtual void OnBeginPlay() {}

		void Destroy();

		// Component creation API
		template <typename ComponentType>
		eastl::shared_ptr<ComponentType> AddComponent();

		template <typename ComponentType>
		eastl::shared_ptr<ComponentType> AddComponent(const TTypeInfo& inTypeInfo);

		// Utility getter and setter functions
		inline bool IsPendingForKill() const { return m_isPendingForKill; }

		UGameWorld& GetWorld();
		const UGameWorld& GetWorld() const;

		inline size_t GetComponentCount() const { return m_actorComponents.size(); }
		inline UGameWorldLayer& GetOwningWorldLayer() { return *m_owningWorldLayer; }
		inline const UGameWorldLayer& GetOwningWorldLayer() const { return *m_owningWorldLayer; }
		inline const eastl::vector<eastl::shared_ptr<UComponent>>& GetEntityComponents() const { return m_actorComponents; }

		template <typename ComponentType>
		eastl::shared_ptr<const ComponentType> GetFirstComponentByType() const;

		template <typename ComponentType>
		eastl::shared_ptr<ComponentType> GetFirstComponentByType();

		inline void SetOwningWorldLayer(UGameWorldLayer& inWorldLayer) { m_owningWorldLayer = &inWorldLayer; }
	private:
		bool m_isPendingForKill;
		UGameWorldLayer* m_owningWorldLayer;
		eastl::vector<eastl::shared_ptr<UComponent>> m_actorComponents;
	};

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

		eastl::shared_ptr<ComponentType> newComponent = inTypeInfo.CreateDefaultObject<ComponentType>();

		newComponent->SetOwner(*this);

		m_actorComponents.push_back(newComponent);

		return newComponent;
	}

	template <typename ComponentType>
	eastl::shared_ptr<const ComponentType> AEntity::GetFirstComponentByType() const
	{
		for (const auto& currentComponent : m_actorComponents)
		{
			if (IsA<ComponentType>(currentComponent))
			{
				return currentComponent;
			}
		}

		return nullptr;
	}

	template <typename ComponentType>
	eastl::shared_ptr<ComponentType> AEntity::GetFirstComponentByType()
	{
		for (const auto& currentComponent : m_actorComponents)
		{
			if (IsA<ComponentType>(currentComponent))
			{
				return currentComponent;
			}
		}

		return nullptr;
	}
}
