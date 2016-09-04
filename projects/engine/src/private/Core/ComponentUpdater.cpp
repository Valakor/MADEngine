#include "Core/ComponentUpdater.h"
#include "Core/Entity.h"
#include "Core/Component.h"

#include <EASTl/algorithm.h>

namespace MAD
{
	const PriorityLevel ComponentUpdater::s_staticPhysicsPriorityLevel = 1000;

	ComponentUpdater::ComponentUpdater() : m_isUpdating(false), m_nextAssignedPriorityLevel(1) {}

	void ComponentUpdater::UpdatePrePhysicsComponents(float inDeltaTime)
	{
		auto currentPriorityLevelIter = m_componentPriorityBlocks.begin();

		while (currentPriorityLevelIter->first < ComponentUpdater::s_staticPhysicsPriorityLevel)
		{
			// Iterate over the entries of the component containers while the priority level is lower than the physics priority level
			for (auto& currentComponent : currentPriorityLevelIter->second.m_blockComponents)
			{
				currentComponent->UpdateComponent(inDeltaTime);
			}
		}
	}

	void ComponentUpdater::UpdatePostPhysicsComponents(float inDeltaTime)
	{
		// Find the first priority level entry that is greater priority than the physics priority level
		const auto priorityLevelEndIter = m_componentPriorityBlocks.end();
		auto currentPriorityLevelIter = m_componentPriorityBlocks.upper_bound(ComponentUpdater::s_staticPhysicsPriorityLevel);

		while (currentPriorityLevelIter != priorityLevelEndIter)
		{
			// Iterate over the rest of the components
			for (auto& currentComponent : currentPriorityLevelIter->second.m_blockComponents)
			{
				currentComponent->UpdateComponent(inDeltaTime);
			}
		}
	}
}