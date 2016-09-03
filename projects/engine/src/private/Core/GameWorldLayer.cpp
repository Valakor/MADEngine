#include "Core/GameWorldLayer.h"
#include "Core/Entity.h"

namespace MAD
{
	UGameWorldLayer::UGameWorldLayer(UGameWorld& inOwningWorld, const eastl::string& inLayerName) : m_isEnabled(true), m_layerName(inLayerName), m_owningWorld(inOwningWorld) {}

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
				currentEntity->Destroy();
			}
		}
	}
}