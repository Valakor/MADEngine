#pragma once

#include <EASTL/array.h>
#include <EASTL/shared_ptr.h>
#include <EASTL/vector.h>
#include <EASTL/set.h>
#include <EASTL/weak_ptr.h>

#include "Core/Component.h"
#include "Misc/Assert.h"

namespace MAD
{
	/*class AEntity;

	// ComponentUpdater will maintain weak references to all of the components in a single World instance. Ticks the components based on their tick type
	// Each World will contain only one ComponentUpdater. Allows easy management of per-World component updates
	class ComponentUpdater
	{
	public:
		using ComponentContainer = eastl::vector<eastl::weak_ptr<UComponent>>;

		template <typename ComponentType, TickType ComponentTickType>
		void AddComponentToTickGroup(eastl::weak_ptr<ComponentType> inCompWeakPtr);

		template <TickType ComponentTickType>
		void UpdateTickGroup(float inDeltaTime);

	private:
		eastl::array<ComponentContainer, static_cast<size_t>(TickType::TT_TickTypeCount)> m_componentTickBuckets; // Number of tick group buckets is known at compile time based on TickType enumeration
	};

	template <typename ComponentType, TickType ComponentTickType>
	void ComponentUpdater::AddComponentToTickGroup(eastl::weak_ptr<ComponentType> inCompWeakPtr)
	{
		MAD_ASSERT_DESC(!inCompWeakPtr.expired(), "Warning: Trying to add an expired weak_ptr to the tick group. Will automatically be taken out upon update!");

		m_componentTickBuckets[static_cast<uint8_t>(ComponentTickType)].emplace_back(inCompWeakPtr);
	}

	template <TickType ComponentTickType>
	void ComponentUpdater::UpdateTickGroup(float inDeltaTime)
	{
		ComponentContainer& tickGroupContainerRef = m_componentTickBuckets[static_cast<uint8_t>(ComponentTickType)];

		// Remove all expired component ptrs first
		tickGroupContainerRef.erase(eastl::remove_if(tickGroupContainerRef.begin(), tickGroupContainerRef.end(), [](eastl::weak_ptr<UComponent> currentCompWeakPtr)
		{
			return currentCompWeakPtr.expired();
		}), tickGroupContainerRef.end());

		for (auto& currentComponent : tickGroupContainerRef)
		{
			currentComponent.lock()->UpdateComponent(inDeltaTime);
		}
	}*/
	using PriorityLevel = uint32_t;

	struct ComponentPriorityBlock
	{
		explicit ComponentPriorityBlock(TypeID inComponentTypeID) : m_blockComponentTypeID(inComponentTypeID) {}

		TypeID m_blockComponentTypeID;
		eastl::vector<eastl::weak_ptr<UComponent>> m_blockComponents;
	};

	class ComponentUpdater
	{
	public:
		using ComponentContainer = eastl::multiset<uint32_t, ComponentPriorityBlock>;

		template <typename ComponentType>
		void AddComponent(eastl::weak_ptr<ComponentType> inCompWeakPtr);

		void UpdatePrePhysicsComponents(float inDeltaTime);
		void UpdatePostPhysicsComponents(float inDeltaTime);
	private:
		ComponentContainer m_componentContainers;
	};

	template <typename ComponentType>
	void ComponentUpdater::AddComponent(eastl::weak_ptr<ComponentType> inCompWeakPtr)
	{
		
	}
}
