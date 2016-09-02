#pragma once

#include <EASTL/shared_ptr.h>
#include <EASTL/vector.h>

#include "Core/Component.h"
#include "Core/Object.h"

namespace MAD
{
	class UGameWorld;

	class AEntity : public UObject
	{
		MAD_DECLARE_ACTOR(AEntity, UObject)

	public:
		explicit AEntity(UGameWorld& owningWorld);

		// [TODO] Probably shouldn't have the user pass in a shared_ptr to attach to the Entity
		// Variadic template for AttachComponent that takes the arguments for the UComponent constructor
		// and creates the shared_ptr WITHIN AttachComponent

		inline void AttachComponent(eastl::shared_ptr<UComponent> inNewComponent) { m_actorComponents.push_back(inNewComponent); }
		
		inline size_t GetComponentCount() const { return m_actorComponents.size(); }
		inline UGameWorld* GetOwningWorld() const { return m_owningWorld; }

		template <typename ComponentType>
		eastl::weak_ptr<const ComponentType> GetFirstComponentByType() const;

		template <typename ComponentType>
		eastl::weak_ptr<ComponentType> GetFirstComponentByType();

	private:
		UGameWorld* m_owningWorld;
		eastl::vector<eastl::shared_ptr<UComponent>> m_actorComponents; // Maintain weak references to the actor's components (ComponentManager is the actual manager of all components)
	};

	template <typename ComponentType>
	eastl::weak_ptr<const ComponentType> AEntity::GetFirstComponentByType() const
	{
		for (const auto& currentComponent : m_actorComponents)
		{
			if (IsA<ComponentType>(currentComponent))
			{
				return eastl::weak_ptr<const ComponentType>(currentComponent);
			}
		}

		return nullptr;
	}

	template <typename ComponentType>
	eastl::weak_ptr<ComponentType> AEntity::GetFirstComponentByType()
	{
		for (const auto& currentComponent : m_actorComponents)
		{
			if (IsA<ComponentType>(currentComponent))
			{
				return eastl::weak_ptr<ComponentType>(currentComponent);
			}
		}

		return nullptr;
	}
}
