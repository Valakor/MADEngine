#pragma once

#include "Core\Object.h"
#include "Core\Component.h"

#include <unordered_set>
#include <memory>
#include <cstdint>

namespace MAD
{
	class UGameWorld;

	class AEntity : public UObject
	{
		MAD_DECLARE_ACTOR(AEntity)
	public:
		explicit AEntity(UGameWorld* owningWorld);

		inline void AttachComponent(std::shared_ptr<UComponent> inNewComponent) { m_actorComponents.emplace(inNewComponent); }

		inline size_t GetComponentCount() const { return m_actorComponents.size(); }
		inline UGameWorld* GetOwningWorld() { return m_owningWorld; }

		template <typename ComponentType>
		std::weak_ptr<const ComponentType> GetFirstComponentByType() const;

		template <typename ComponentType>
		std::weak_ptr<ComponentType> GetFirstComponentByType();
	private:
		UGameWorld* m_owningWorld;
		std::unordered_set<std::shared_ptr<UComponent>> m_actorComponents; // Maintain weak references to the actor's components (ComponentManager is the actual manager of all components)
	};

	template <typename ComponentType>
	std::weak_ptr<const ComponentType> AEntity::GetFirstComponentByType() const
	{
		for (const auto& currentComponent : m_actorComponents)
		{
			if (IsA<ComponentType>(currentComponent))
			{
				return std::weak_ptr<const ComponentType>(currentComponent);
			}
		}

		return nullptr;
	}

	template <typename ComponentType>
	std::weak_ptr<ComponentType> AEntity::GetFirstComponentByType()
	{
		for (const auto& currentComponent : m_actorComponents)
		{
			if (IsA<ComponentType>(currentComponent))
			{
				return std::weak_ptr<ComponentType>(currentComponent);
			}
		}

		return nullptr;
	}
}