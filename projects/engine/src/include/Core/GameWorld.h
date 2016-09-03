#pragma once

#include <EASTL/shared_ptr.h>
#include <EASTL/type_traits.h>
#include <EASTL/hash_map.h>
#include <EASTL/string.h>

#include "Core/Object.h"
#include "Core/GameWorldLayer.h"
#include "Core/ComponentUpdater.h"

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
		explicit UGameWorld(const eastl::string& inDefaultLayerName = UGameWorld::s_defaultWorldLayerName);
		virtual ~UGameWorld() { }

		// Allows you spawn an Entity in a different World Layer than current one
		template <typename EntityType, typename ...Args>
		eastl::weak_ptr<EntityType> SpawnEntity(Args&&... inConstructorArgs);

		template <typename EntityType, typename ...Args>
		eastl::weak_ptr<EntityType> SpawnEntity(const eastl::string& inWorldLayerName, Args&&... inConstructorArgs);

		// Removes any entities (and their components) that pending to be killed
		void CleanupEntities();
		
		inline ComponentUpdater& GetComponentUpdater() { return m_componentUpdater; }
		
		void UpdatePrePhysics(float inDeltaTime);
		void UpdatePostPhysics(float inDeltaTime);
	private:
		const eastl::string m_defaultLayerName;
		WorldLayerContainer m_worldLayers;
		ComponentUpdater m_componentUpdater;
	};

	template <typename EntityType, typename ...Args>
	eastl::weak_ptr<EntityType> UGameWorld::SpawnEntity(Args&&... inConstructorArgs)
	{
		return SpawnEntity<EntityType>(m_defaultLayerName);
	}

	template <typename EntityType, typename ...Args>
	eastl::weak_ptr<EntityType> UGameWorld::SpawnEntity(const eastl::string& inWorldLayerName, Args&&... inConstructorArgs)
	{
		static_assert(eastl::is_base_of<AEntity, EntityType>::value, "ActorType must be a derived type of AActor");

		WorldLayerContainer::iterator targetWorldLayerIter;

		auto targetWorldLayerIter = m_worldLayers.find(inWorldLayerName);

		if (targetWorldLayerIter == m_worldLayers.end())
		{
			// No world layer with that name yet, create one and add the new Entity to it
			targetWorldLayerIter = m_worldLayers.emplace(eastl::make_pair(inWorldLayerName, UGameWorldLayer(*this, inWorldLayerName)));
		}

		return targetWorldLayerIter->second.SpawnEntityToLayer<EntityType>(eastl::forward<Args>(inConstructorArgs)...);
	}
}
