#pragma once

#include <EASTL/shared_ptr.h>
#include <EASTL/type_traits.h>
#include <EASTL/hash_map.h>
#include <EASTL/string.h>
#include <EASTL/type_traits.h>

#include "Core/Object.h"
#include "Core/GameWorldLayer.h"
#include "Core/ComponentUpdater.h"
#include "Core/ValueLockAdapter.h"
#include "Misc/Logging.h"

namespace MAD
{
	class AEntity;

	class UGameWorld : public UObject
	{
		MAD_DECLARE_CLASS(UGameWorld, UObject)
	public:
		static const eastl::string s_defaultWorldLayerName;

		using WorldLayerContainer = eastl::hash_map<eastl::string, UGameWorldLayer>;
	public:
		UGameWorld();

		virtual ~UGameWorld() { }

		template <typename EntityType>
		eastl::shared_ptr<EntityType> SpawnEntity();

		template <typename EntityType>
		eastl::shared_ptr<EntityType> SpawnEntity(const eastl::string& inWorldLayerName);
		
		template <typename CommonAncestorEntityType>
		eastl::shared_ptr<CommonAncestorEntityType> SpawnEntity(const TTypeInfo& inTypeInfo, const eastl::string& inWorldLayerName);
		
		const eastl::string& GetWorldName() const { return m_worldName; }
		const eastl::string& GetDefaultLayerName() const { return m_defaultLayerName.GetValue(); }
		void SetWorldName(const eastl::string& inWorldName) { if (!inWorldName.empty()) m_worldName = inWorldName; }
		void SetDefaultLayerName(const eastl::string& inDefaultLayerName) { m_defaultLayerName.SetValue(inDefaultLayerName); }
		void LockDefaults() { m_defaultLayerName.LockValue(); } // Lock the default value so it can no longer be changed

		// Removes any entities (and their components) that pending to be killed
		void CleanupEntities();
		
		inline ComponentUpdater& GetComponentUpdater() { return m_componentUpdater; }
		inline const ComponentUpdater& GetComponentUpdater() const { return m_componentUpdater; }

		void UpdatePrePhysics(float inDeltaTime);
		void UpdatePostPhysics(float inDeltaTime);
	private:
		void RegisterEntity(AEntity& inEntity, UGameWorldLayer& inWorldLayer); // Registers the entity to the world layer and registers the entity's components to the component updater
	private:
		eastl::string m_worldName;
		ValueLockAdapater<eastl::string> m_defaultLayerName;
		WorldLayerContainer m_worldLayers;
		ComponentUpdater m_componentUpdater;
	};

	template <typename EntityType>
	eastl::shared_ptr<EntityType> UGameWorld::SpawnEntity()
	{
		static_assert(eastl::is_base_of<AEntity, EntityType>::value, "Error: You may only create entities that are of type AEntity or more derived"); // Make sure the user is only specifying children classes of AEntity

		return SpawnEntity<EntityType>(m_defaultLayerName.GetValue());
	}

	template <typename EntityType>
	eastl::shared_ptr<EntityType> UGameWorld::SpawnEntity(const eastl::string& inWorldLayerName)
	{
		static_assert(eastl::is_base_of<AEntity, EntityType>::value, "Error: You may only create entities that are of type AEntity or more derived"); // Make sure the user is only specifying children classes of AEntity

		return SpawnEntity<EntityType>(*EntityType::StaticClass(), inWorldLayerName);
	}

	template <typename CommonAncestorEntityType>
	eastl::shared_ptr<CommonAncestorEntityType> UGameWorld::SpawnEntity(const TTypeInfo& inTypeInfo, const eastl::string& inWorldLayerName)
	{
		static_assert(eastl::is_base_of<AEntity, CommonAncestorEntityType>::value, "Error: You may only create entities that are of type AEntity or more derived"); // Make sure the user is only specifying children classes of AEntity

		auto targetWorldLayerIter = m_worldLayers.find(inWorldLayerName);
		
		if (targetWorldLayerIter == m_worldLayers.end())
		{
			// Create new world layer and assign its owning world
			targetWorldLayerIter = m_worldLayers.emplace(inWorldLayerName).first;
			targetWorldLayerIter->second.SetWorldLayerName(inWorldLayerName);
			targetWorldLayerIter->second.SetOwningWorld(*this);
		}

		LOG(LogDefault, Log, "Spawning Entity at Layer: %s", targetWorldLayerIter->first.c_str());

		// Create default EntityType object through common creation API and assign the entity's owning world layer
		eastl::shared_ptr<CommonAncestorEntityType> defaultEntityObject = inTypeInfo.CreateDefaultObject<CommonAncestorEntityType>();
		
		//Add entity to the layer's entity list
		targetWorldLayerIter->second.AddEntityToLayer(defaultEntityObject);

		// Since we defer the registration of the entity's owning GameWorldLayer and it's owning GameWorld, you cannot access these within the constructor of the entity
		RegisterEntity(*defaultEntityObject, targetWorldLayerIter->second);

		defaultEntityObject->OnBeginPlay();

		return defaultEntityObject;
	}
}
