#pragma once

#include "Engine.h"
#include "Core\Component.h"
#include "Misc\Assert.h"

#include <array>
#include <vector>
#include <type_traits>
#include <algorithm>

namespace MAD
{
	class AEntity;

	// ComponentUpdater will maintain weak references to all of the components in a single World instance. Ticks the components based on their tick type
	// Each World will contain only one ComponentUpdater. Allows easy management of per-World component updates
	class ComponentUpdater
	{
	public:
		using ComponentContainer = std::vector<std::weak_ptr<UComponent>>;
	public:
		template <typename ComponentType, TickType ComponentTickType>
		void AddComponentToTickGroup(std::weak_ptr<ComponentType> inCompWeakPtr);

		template <TickType ComponentTickType>
		void UpdateTickGroup(float inDeltaTime);

	private:
		array<ComponentContainer, static_cast<size_t>(TickType::TT_TickTypeCount)> m_componentTickBuckets; // Number of tick group buckets is known at compile time based on TickType enumeration
	};

	template <typename ComponentType, TickType ComponentTickType>
	void ComponentUpdater::AddComponentToTickGroup(std::weak_ptr<ComponentType> inCompWeakPtr)
	{
		MAD_ASSERT_DESC(!inCompWeakPtr.expired(), "Warning: Trying to add an expired weak_ptr to the tick group. Will automatically be taken out upon update!");

		m_componentTickBuckets[static_cast<uint8_t>(ComponentTickType)].emplace_back(inCompWeakPtr);
	}

	template <TickType ComponentTickType>
	void ComponentUpdater::UpdateTickGroup(float inDeltaTime)
	{
		ComponentContainer& tickGroupContainerRef = m_componentTickBuckets[static_cast<uint8_t>(ComponentTickType)];

		// Remove all expired component ptrs first
		tickGroupContainerRef.erase(std::remove_if(tickGroupContainerRef.begin(), tickGroupContainerRef.end(), [](std::weak_ptr<UComponent> currentCompWeakPtr)
		{
			return currentCompWeakPtr.expired();
		}), tickGroupContainerRef.end());

		for (auto& currentComponent : tickGroupContainerRef)
		{
			currentComponent.lock()->UpdateComponent(inDeltaTime);
		}
	}
}