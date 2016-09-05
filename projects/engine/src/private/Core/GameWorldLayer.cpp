#include "Core/GameWorldLayer.h"
#include "Core/Entity.h"

namespace MAD
{
	UGameWorldLayer::~UGameWorldLayer()
	{
		CleanupOwnedEntities();
	}

	void UGameWorldLayer::CleanupOwnedEntities()
	{
		for (auto& currentEntity : m_layerEntities)
		{
			if (currentEntity->IsPendingForKill())
			{
				// Remove the entity's components from the component updater
			}
		}

		// After removing the components, remove all entities that are marked for kill
	}
}