#pragma once

#include "Core/Object.h"

#include <EASTL/vector.h>
#include <EASTL/shared_ptr.h>
#include <EASTL/string.h>

namespace MAD
{
	class UGameWorld;
	class AEntity;

	class UGameWorldLayer : public UObject
	{
		MAD_DECLARE_CLASS(UGameWorldLayer, UObject)
	public:
		virtual ~UGameWorldLayer();

		void CleanupOwnedEntities();

		// Utility function that allows you to spawn an Entity within the current world layer of the spawner
		template <typename EntityType, typename ...Args>
		eastl::weak_ptr<EntityType> SpawnEntityToLayer(Args&&... inConstructorArgs);

		// TODO: Allow for activating/de-activating an entire layer, for now will only support completely removing entities

		inline void SetOwningWorld(UGameWorld& inGameWorld) { m_owningWorld = &inGameWorld; }
		inline void SetWorldLayerName(const eastl::string& inLayerName) { m_layerName = inLayerName; }

		inline UGameWorld& GetOwningWorld() { return *m_owningWorld; }
		inline const eastl::string& GetLayerName() const { return m_layerName; }
	private:
		bool m_isEnabled;
		UGameWorld* m_owningWorld;
		eastl::string m_layerName;
		eastl::vector<eastl::shared_ptr<AEntity>> m_layerEntities;
	};

	template <typename EntityType, typename ...Args>
	eastl::weak_ptr<EntityType> UGameWorldLayer::SpawnEntityToLayer(Args&&... inConstructorArgs)
	{
		eastl::shared_ptr<EntityType> newEntityPtr = eastl::make_shared<EntityType>(eastl::forward<Args>(inConstructorArgs)...);

		m_layerEntities.emplace_back(newEntityPtr);

		return newEntityPtr;
	}

}