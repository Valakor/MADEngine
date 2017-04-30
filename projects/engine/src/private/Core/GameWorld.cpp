#include "Core/GameWorld.h"

#include "Core/GameEngine.h"
#include "Core/Entity.h"
#include "Misc/Logging.h"
#include "Misc/Remotery.h"

namespace MAD
{
	const eastl::string OGameWorld::s_defaultWorldLayerName = "Default_Layer";

	OGameWorld::OGameWorld(OGameWorld* inOwningGameWorld)
		: Super_t(inOwningGameWorld)
		, m_defaultLayerName(s_defaultWorldLayerName) {}

	OGameWorld::~OGameWorld()
	{
		for (auto& layer : m_worldLayers)
		{
			layer.second.CleanupExpiredEntities();
		}

		m_worldLayers.clear();
	}

	void OGameWorld::FinalizeSpawnEntity(eastl::shared_ptr<AEntity> inEntity)
	{
		MAD_ASSERT_DESC(inEntity.get() != nullptr, "Cannot finalize spawning a null entity");

		LOG(LogDefault, Log, "Finalizing spawning entity of type %s at Layer: %s\n", inEntity->GetTypeInfo()->GetTypeName(), inEntity->GetOwningWorldLayer().GetLayerName().c_str());

		inEntity->GetOwningWorldLayer().AddEntityToLayer(inEntity);
		inEntity->PostInitialize();
		inEntity->BeginPlay();
	}

	size_t OGameWorld::GetEntityCount() const
	{
		size_t resultEntityCount = 0;

		for (const auto& currentWorldLayer : m_worldLayers)
		{
			resultEntityCount += currentWorldLayer.second.GetEntityCount();
		}

		return resultEntityCount;
	}

	void OGameWorld::CleanupEntities()
	{
		rmt_ScopedCPUSample(World_CleanupEntities, 0);

		//LOG(LogDefault, Log, "Cleaning up entites from %s\n", m_worldName.c_str());

		// Cleans up the entities that are pending for kill
		for (auto& currentWorldLayer : m_worldLayers)
		{
			currentWorldLayer.second.CleanupExpiredEntities();
		}
	}

	void OGameWorld::UpdatePrePhysics(float inDeltaTime)
	{
		rmt_ScopedCPUSample(World_UpdatePrePhysics, 0);
		m_componentUpdater.UpdatePrePhysicsComponents(inDeltaTime);
	}

	void OGameWorld::UpdatePostPhysics(float inDeltaTime)
	{
		rmt_ScopedCPUSample(World_UpdatePostPhysics, 0);
		m_componentUpdater.UpdatePostPhysicsComponents(inDeltaTime);
	}
}
