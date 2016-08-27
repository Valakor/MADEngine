#pragma once

#include "Core\Object.h"
#include "Core\ComponentUpdater.h"

#include <type_traits>
#include <unordered_set>

namespace MAD
{
	class AEntity;

	class UGameWorld : public UObject
	{
		MAD_DECLARE_CLASS(UGameWorld)
	public:
		virtual void Update(float inDeltaTime);

		template <typename ActorType>
		std::weak_ptr<ActorType> SpawnActor();

		inline ComponentUpdater& GetComponentUpdater() { return m_componentUpdater; }
	private:
		std::unordered_set<std::shared_ptr<AEntity>> m_ownedActors;
		ComponentUpdater m_componentUpdater;
	};

	template <typename ActorType>
	std::weak_ptr<ActorType> UGameWorld::SpawnActor()
	{
		static_assert(std::is_base_of<AEntity, ActorType>::value, "ActorType must be a derived type of AActor");

		std::shared_ptr<ActorType> newActorPtr = std::make_shared<ActorType>(this);

		m_ownedActors.emplace(newActorPtr);

		return newActorPtr;
	}
}