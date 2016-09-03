#pragma once

#include <EASTL/shared_ptr.h>
#include <EASTL/vector.h>

#include "Core/Component.h"
#include "Core/Object.h"

namespace MAD
{
	class UGameWorldLayer;

	class AEntity : public UObject
	{
		MAD_DECLARE_ACTOR(AEntity, UObject)

	public:
		explicit AEntity(UGameWorldLayer& inOwningWorldLayer);

		inline void AttachComponent(eastl::weak_ptr<UComponent> inNewComponent) { m_actorComponents.push_back(inNewComponent); }

		void Destroy();

		inline bool IsPendingForKill() const { return m_isPendingForKill; }
		inline size_t GetComponentCount() const { return m_actorComponents.size(); }

		inline UGameWorldLayer& GetOwningWorldLayer() { return m_owningWorldLayer; }
		inline const UGameWorldLayer& GetOwningWorldLayer() const { return m_owningWorldLayer; }

		template <typename ComponentType>
		eastl::weak_ptr<const ComponentType> GetFirstComponentByType() const;

		template <typename ComponentType>
		eastl::weak_ptr<ComponentType> GetFirstComponentByType();

	private:
		bool m_isPendingForKill;
		UGameWorldLayer& m_owningWorldLayer;
		eastl::vector<eastl::weak_ptr<UComponent>> m_actorComponents; // Maintain weak references to the actor's components (ComponentManager is the actual manager of all components)
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
