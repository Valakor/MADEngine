#pragma once

#include <EASTL/shared_ptr.h>
#include <EASTL/type_traits.h>
#include <EASTL/vector.h>

#include "Core/Object.h"
#include "Core/ComponentUpdater.h"

namespace MAD
{
	class AEntity;

	class UGameWorld : public UObject
	{
		MAD_DECLARE_CLASS(UGameWorld, UObject)

	public:
		virtual ~UGameWorld() { }
		virtual void Update(float inDeltaTime);

		template <typename ActorType>
		eastl::weak_ptr<ActorType> SpawnActor();

		inline ComponentUpdater& GetComponentUpdater() { return m_componentUpdater; }

	private:
		eastl::vector<eastl::shared_ptr<AEntity>> m_ownedActors;
		ComponentUpdater m_componentUpdater;
	};

	template <typename ActorType>
	eastl::weak_ptr<ActorType> UGameWorld::SpawnActor()
	{
		static_assert(eastl::is_base_of<AEntity, ActorType>::value, "ActorType must be a derived type of AActor");

		eastl::shared_ptr<ActorType> newActorPtr = eastl::make_shared<ActorType>(this);
		m_ownedActors.push_back(newActorPtr);

		return newActorPtr;
	}
}
