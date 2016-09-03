#include "Core/GameWorld.h"

#include "Core/GameEngine.h"
#include "Core/Entity.h"

namespace MAD
{
	const eastl::string UGameWorld::s_defaultWorldLayerName = "Default_Layer";

	UGameWorld::UGameWorld(const eastl::string& inDefaultLayerName /*= UGameWorld::s_defaultWorldLayerName*/) : m_defaultLayerName(inDefaultLayerName) {}

	void UGameWorld::CleanupEntities()
	{
		// Cleans up the entities that are pending for kill
		for (auto& currentWorldLayer : m_worldLayers)
		{
			currentWorldLayer.second.CleanupOwnedEntities();
		}
	}

	void UGameWorld::UpdatePrePhysics(float inDeltaTime)
	{
		m_componentUpdater.UpdatePrePhysicsComponents(inDeltaTime);
	}

	void UGameWorld::UpdatePostPhysics(float inDeltaTime)
	{
		m_componentUpdater.UpdatePostPhysicsComponents(inDeltaTime);
	}

}