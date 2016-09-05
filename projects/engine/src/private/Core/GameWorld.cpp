#include "Core/GameWorld.h"

#include "Core/GameEngine.h"
#include "Core/Entity.h"
#include "Misc/Logging.h"

namespace MAD
{
	const eastl::string UGameWorld::s_defaultWorldLayerName = "Default_Layer";

	UGameWorld::UGameWorld() : m_defaultLayerName(s_defaultWorldLayerName) {}

	void UGameWorld::CleanupEntities()
	{
		LOG(LogDefault, Log, "Cleaning up entites from %s\n", m_worldName.c_str());

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

	void UGameWorld::RegisterEntity(AEntity& inEntity, UGameWorldLayer& inWorldLayer)
	{
		// Register entity to world layer
		inEntity.SetOwningWorldLayer(inWorldLayer);

		// Register entity's components to the component updater
		const auto& entityComponents = inEntity.GetEntityComponents();
		
		for (const auto& currentComponent : entityComponents)
		{
			m_componentUpdater.RegisterComponent(currentComponent);
		}
	}

}