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

	void OGameWorld::FinalizeSpawnEntity(eastl::shared_ptr<AEntity> inEntity)
	{
		MAD_ASSERT_DESC(inEntity.get() != nullptr, "Cannot finalize spawning a null entity");

		LOG(LogDefault, Log, "Finalizing spawning entity of type %s at Layer: %s\n", inEntity->GetTypeInfo()->GetTypeName(), inEntity->GetOwningWorldLayer().GetLayerName().c_str());

		inEntity->GetOwningWorldLayer().AddEntityToLayer(inEntity);
		inEntity->PostInitialize();
		inEntity->BeginPlay();
	}

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
}
