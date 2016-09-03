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
		// Iterate over the entries of the component containers while the priority level is lower than the physics priority level
		
		for (const auto& currentPriorityIter : m_prePhysicsPriorityBlocks)
		{
			for (auto& currentCompIter : currentPriorityIter.second.m_blockComponents)
			{
				if (!currentCompIter->GetOwner().IsPendingForKill())
				{
					currentCompIter->UpdateComponent(inDeltaTime);
				}
			}
		}
	}

	void ComponentUpdater::UpdatePostPhysicsComponents(float inDeltaTime)
	{
		// Iterate over the entries of the component containers from the physics priority level end of the container
		
		for (const auto& currentPriorityIter : m_postPhysicsPriorityBlocks)
		{
			for (auto& currentCompIter : currentPriorityIter.second.m_blockComponents)
			{
				if (!currentCompIter->GetOwner().IsPendingForKill())
				{
					currentCompIter->UpdateComponent(inDeltaTime);
				}
			}
		}
	}

}