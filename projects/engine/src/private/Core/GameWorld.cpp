#include "Core/GameWorld.h"

#include "Core/GameEngine.h"
#include "Core/Entity.h"
#include "Misc/Logging.h"

namespace MAD
{
	const eastl::string OGameWorld::s_defaultWorldLayerName = "Default_Layer";

	OGameWorld::OGameWorld(OGameWorld* inOwningGameWorld)
		: Super(inOwningGameWorld)
		, m_defaultLayerName(s_defaultWorldLayerName) {}

	void OGameWorld::CleanupEntities()
	{
		//LOG(LogDefault, Log, "Cleaning up entites from %s\n", m_worldName.c_str());

		// Cleans up the entities that are pending for kill
		for (auto& currentWorldLayer : m_worldLayers)
		{
			currentWorldLayer.second.CleanupExpiredEntities();
		}
	}

	void OGameWorld::UpdatePrePhysics(float inDeltaTime)
	{
		m_componentUpdater.UpdatePrePhysicsComponents(inDeltaTime);
	}

	void OGameWorld::UpdatePostPhysics(float inDeltaTime)
	{
		m_componentUpdater.UpdatePostPhysicsComponents(inDeltaTime);
	}

	void OGameWorld::RegisterEntity(AEntity& inEntity, OGameWorldLayer& inWorldLayer)
	{
		// Register entity to world layer
		inEntity.SetOwningWorldLayer(inWorldLayer);
	}

}
