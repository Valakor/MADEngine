#include "Core/ComponentUpdater.h"
#include "Core/Entity.h"
#include "Core/Component.h"
#include "Misc/Logging.h"

#include <EASTl/algorithm.h>

namespace MAD
{
	DECLARE_LOG_CATEGORY(LogComponentUpdater);

	ComponentUpdater::ComponentUpdater() : m_isUpdating(false) {}

	void ComponentUpdater::RemoveComponent(eastl::shared_ptr<UComponent> inTargetComponent)
	{
		// Before removing anything, we need to make sure that we aren't in the middle of updating (or else we'll invalidate a lot of iterators)
		if (!m_isUpdating)
		{
			// Get the priority level of the component
			TComponentPriorityInfo* componentPriorityInfo = inTargetComponent->GetPriorityInfo();
			const TypeID componentTypeID = inTargetComponent->GetTypeInfo()->GetTypeID();
			const PriorityLevel componentPriorityLevel = componentPriorityInfo->GetPriorityLevel(); // WARNING: Assumes that priority levels are not being changed after a component is added. Priority levels should only be changed before main loop begins

			auto priorityLevelFindIter = m_componentPriorityBlocks.equal_range(componentPriorityLevel);

			while (priorityLevelFindIter.first != priorityLevelFindIter.second)
			{
				if (priorityLevelFindIter.first->second.m_blockComponentTypeID == componentTypeID)
				{
					// Found the priority block for the target component to be deleted. We just need to delete it now
					ComponentPriorityBlock::ComponentContainer& targetComponentContainer = priorityLevelFindIter.first->second.m_blockComponents;

					targetComponentContainer.erase(eastl::remove(targetComponentContainer.begin(), targetComponentContainer.end(), inTargetComponent), targetComponentContainer.end());

					return;
				}

				++priorityLevelFindIter.first;
			}
		}
	}

	void ComponentUpdater::UpdatePrePhysicsComponents(float inDeltaTime)
	{
		auto currentPriorityLevelIter = m_componentPriorityBlocks.begin();
		const auto priorityLevelEndIter = m_componentPriorityBlocks.end();

		while (currentPriorityLevelIter->first < EPriorityLevelReference::EPriorityLevel_Physics && currentPriorityLevelIter != priorityLevelEndIter)
		{
			LOG(LogComponentUpdater, Log, "Updating priority %d\n", currentPriorityLevelIter->first);
			// Iterate over the entries of the component containers while the priority level is lower than the physics priority level
			
			for (auto& currentComponent : currentPriorityLevelIter->second.m_blockComponents)
			{
				// Only update the component if it's owner hasn't been marked for kill
				if (!currentComponent->GetOwner().IsPendingForKill())
				{
					currentComponent->UpdateComponent(inDeltaTime);
				}
			}

			++currentPriorityLevelIter;
		}
	}

	void ComponentUpdater::UpdatePostPhysicsComponents(float inDeltaTime)
	{
		// Find the first priority level entry that is greater priority than the physics priority level
		auto currentPriorityLevelIter = m_componentPriorityBlocks.upper_bound(EPriorityLevelReference::EPriorityLevel_Physics);
		const auto priorityLevelEndIter = m_componentPriorityBlocks.end();

		while (currentPriorityLevelIter != priorityLevelEndIter)
		{
			LOG(LogComponentUpdater, Log, "Updating priority %d\n", currentPriorityLevelIter->first);

			// Iterate over the rest of the components
			for (auto& currentComponent : currentPriorityLevelIter->second.m_blockComponents)
			{
				if (!currentComponent->GetOwner().IsPendingForKill())
				{
					currentComponent->UpdateComponent(inDeltaTime);
				}
			}

			++currentPriorityLevelIter;
		}

		LOG(LogComponentUpdater, Log, "\n\n\n");
	}

	void ComponentUpdater::RegisterComponent(eastl::shared_ptr<UComponent> inNewComponentPtr)
	{
		TComponentPriorityInfo* componentPriorityInfo = inNewComponentPtr->GetPriorityInfo();
		const PriorityLevel componentPriorityLevel = componentPriorityInfo->GetPriorityLevel();
		const TypeID componentTypeID = inNewComponentPtr->GetTypeInfo()->GetTypeID();

		auto priorityBlockFindIter = m_componentPriorityBlocks.equal_range(componentPriorityLevel);

		// Since there can be multiple priority blocks with the same priority, we need to find the one that has the component ID that
		// corresponds with the component type we're trying to add

		while (priorityBlockFindIter.first != priorityBlockFindIter.second)
		{
			if (priorityBlockFindIter.first->second.m_blockComponentTypeID == componentTypeID)
			{
				priorityBlockFindIter.first->second.m_blockComponents.emplace_back(inNewComponentPtr);
				return;
			}

			++priorityBlockFindIter.first;
		}

		// If we get here, that means this is the first time we're trying to add a component of this priority level
		auto priorityBlockInsertIter = m_componentPriorityBlocks.emplace(componentPriorityLevel);

		priorityBlockInsertIter->second.m_blockComponentTypeID = componentTypeID;
		priorityBlockInsertIter->second.m_blockComponents.emplace_back(inNewComponentPtr);
	}
}