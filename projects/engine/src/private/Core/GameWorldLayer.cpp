#include "Core/GameWorldLayer.h"
#include "Core/Entity.h"

#include "Misc/Logging.h"

#include <EASTL/algorithm.h>

namespace MAD
{
	UGameWorldLayer::~UGameWorldLayer()
	{
		CleanupExpiredEntities();
	}

	void UGameWorldLayer::CleanupExpiredEntities()
	{
		auto entityRemovePredicate = [](eastl::shared_ptr<AEntity> inCurrentEntity) { return inCurrentEntity->IsPendingForKill(); };

		LOG(LogDefault, Log, "Num Entities Before Cleanup: %d\n", m_layerEntities.size());

		for (auto& currentEntity : m_layerEntities)
		{
			if (currentEntity->IsPendingForKill())
			{
				// Remove the entity's components from the component updater
				AEntity::ComponentContainer entityComponents;
				
				currentEntity->GetEntityComponents(entityComponents);

				for (const auto& currentComponent : entityComponents)
				{
					m_owningWorld->GetComponentUpdater().RemoveComponent(currentComponent.lock());
				}
			}
		}

		// After removing the components, remove all entities that are marked for kill from the layer
		m_layerEntities.erase(eastl::remove_if(m_layerEntities.begin(), m_layerEntities.end(), entityRemovePredicate), m_layerEntities.end());

		LOG(LogDefault, Log, "Num Entities After Cleanup: %d\n", m_layerEntities.size());
	}
}
