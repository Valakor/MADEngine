#pragma once


#include "Core/Object.h"
#include "Core/GameWorldLayer.h"
#include "Core/ComponentUpdater.h"
#include "Misc/Logging.h"

#include <EASTL/shared_ptr.h>
#include <EASTL/type_traits.h>
#include <EASTL/hash_map.h>
#include <EASTL/string.h>
#include <EASTL/type_traits.h>

namespace MAD
{
	class AEntity;

	class OGameWorld : public UObject
	{
		MAD_DECLARE_CLASS(OGameWorld, UObject)
	public:
		static const eastl::string s_defaultWorldLayerName;

		using WorldLayerContainer_t = eastl::hash_map<eastl::string, OGameWorldLayer>;
	public:
		explicit OGameWorld(OGameWorld* inOwningGameWorld);

		virtual ~OGameWorld();

		template <typename EntityType>
		eastl::shared_ptr<EntityType> SpawnEntity();

		template <typename EntityType>
		eastl::shared_ptr<EntityType> SpawnEntity(const eastl::string& inWorldLayerName);
		
		template <typename CommonAncestorEntityType>
		eastl::shared_ptr<CommonAncestorEntityType> SpawnEntity(const TTypeInfo& inTypeInfo, const eastl::string& inWorldLayerName);

		// Doesn't add the entity to the layer, doesn't call PostInitializeComponents, etc. Call FinalizeSpawnEntity(...) with the entity to finalize the process
		template <typename CommonAncestorEntityType>
		eastl::shared_ptr<CommonAncestorEntityType> SpawnEntityDeferred(const TTypeInfo& inTypeInfo, const eastl::string& inWorldLayerName);

		void FinalizeSpawnEntity(eastl::shared_ptr<AEntity> inEntity);
		
		const eastl::string& GetWorldName() const { return m_worldName; }
		const eastl::string& GetWorldRelativePath() const { return m_worldRelativePath; }
		const eastl::string& GetDefaultLayerName() const { return m_defaultLayerName; }
		size_t GetEntityCount() const;

		void SetWorldName(const eastl::string& inWorldName) { if (!inWorldName.empty()) m_worldName = inWorldName; }
		void SetWorldRelativePath(const eastl::string& inWorldRelativePath) { if (!inWorldRelativePath.empty()) m_worldRelativePath = inWorldRelativePath; }
		void SetDefaultLayerName(const eastl::string& inDefaultLayerName) { m_defaultLayerName = inDefaultLayerName; }

		// Removes any entities (and their components) that pending to be killed
		void CleanupEntities();
		
		inline UComponentUpdater& GetComponentUpdater() { return m_componentUpdater; }
		inline const UComponentUpdater& GetComponentUpdater() const { return m_componentUpdater; }

		void UpdatePrePhysics(float inDeltaTime);
		void UpdatePostPhysics(float inDeltaTime);
	private:
		eastl::string m_worldName;
		eastl::string m_worldRelativePath;
		eastl::string m_defaultLayerName;
		WorldLayerContainer_t m_worldLayers;
		UComponentUpdater m_componentUpdater;
	};

	template <typename EntityType>
	eastl::shared_ptr<EntityType> OGameWorld::SpawnEntity()
	{
		static_assert(eastl::is_base_of<AEntity, EntityType>::value, "Error: You may only create entities that are of type AEntity or more derived"); // Make sure the user is only specifying children classes of AEntity

		return SpawnEntity<EntityType>(m_defaultLayerName);
	}

	template <typename EntityType>
	eastl::shared_ptr<EntityType> OGameWorld::SpawnEntity(const eastl::string& inWorldLayerName)
	{
		static_assert(eastl::is_base_of<AEntity, EntityType>::value, "Error: You may only create entities that are of type AEntity or more derived"); // Make sure the user is only specifying children classes of AEntity

		return SpawnEntity<EntityType>(*EntityType::StaticClass(), inWorldLayerName);
	}

	template <typename CommonAncestorEntityType>
	eastl::shared_ptr<CommonAncestorEntityType> OGameWorld::SpawnEntity(const TTypeInfo& inTypeInfo, const eastl::string& inWorldLayerName)
	{
		static_assert(eastl::is_base_of<AEntity, CommonAncestorEntityType>::value, "Error: You may only create entities that are of type AEntity or more derived"); // Make sure the user is only specifying children classes of AEntity

		auto targetWorldLayerIter = m_worldLayers.find(inWorldLayerName);
		
		if (targetWorldLayerIter == m_worldLayers.end())
		{
			// Create new world layer and assign its owning world
			targetWorldLayerIter = m_worldLayers.emplace(inWorldLayerName, OGameWorldLayer(this)).first;
			targetWorldLayerIter->second.SetWorldLayerName(inWorldLayerName);
		}

		LOG(LogDefault, Log, "Spawning Entity of type %s at Layer: %s\n", inTypeInfo.GetTypeName(), targetWorldLayerIter->first.c_str());

		// Create default EntityType object through common creation API and assign the entity's owning world layer
		eastl::shared_ptr<CommonAncestorEntityType> defaultEntityObject = CreateDefaultObject<CommonAncestorEntityType>(inTypeInfo, this);
		
		targetWorldLayerIter->second.AddEntityToLayer(defaultEntityObject);

		defaultEntityObject->SetOwningWorldLayer(targetWorldLayerIter->second);

		defaultEntityObject->PostInitialize();
	
		defaultEntityObject->BeginPlay();

		return defaultEntityObject;
	}

	template <typename CommonAncestorEntityType>
	eastl::shared_ptr<CommonAncestorEntityType> OGameWorld::SpawnEntityDeferred(const TTypeInfo& inTypeInfo, const eastl::string& inWorldLayerName)
	{
		static_assert(eastl::is_base_of<AEntity, CommonAncestorEntityType>::value, "Error: You may only create entities that are of type AEntity or more derived"); // Make sure the user is only specifying children classes of AEntity

		auto targetWorldLayerIter = m_worldLayers.find(inWorldLayerName);

		if (targetWorldLayerIter == m_worldLayers.end())
		{
			// Create new world layer and assign its owning world
			targetWorldLayerIter = m_worldLayers.emplace(inWorldLayerName, OGameWorldLayer(this)).first;
			targetWorldLayerIter->second.SetWorldLayerName(inWorldLayerName);
		}

		LOG(LogDefault, Log, "Spawning deferred Entity of type %s at Layer: %s\n", inTypeInfo.GetTypeName(), targetWorldLayerIter->first.c_str());

		// Create default EntityType object through common creation API and assign the entity's owning world layer
		eastl::shared_ptr<CommonAncestorEntityType> defaultEntityObject = CreateDefaultObject<CommonAncestorEntityType>(inTypeInfo, this);;

		defaultEntityObject->SetOwningWorldLayer(targetWorldLayerIter->second);

		return defaultEntityObject;
	}
}
