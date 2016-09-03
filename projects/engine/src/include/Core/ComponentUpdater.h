#pragma once

#include <EASTL/array.h>
#include <EASTL/shared_ptr.h>
#include <EASTL/vector.h>
#include <EASTL/map.h>
#include <EASTL/weak_ptr.h>

#include "Core/Component.h"
#include "Core/ComponentPriorityInfo.h"
#include "Misc/Assert.h"

namespace MAD
{
	// A ComponentPriorityBlock represents a set of components of the same type and same priority. Having one block for each component type allows us
	// to guarantee update order across component types (i.e if I give TransformComponent a higher priority than CameraComponent, it's guaranteed
	// that all TransformComponents will update before all CameraComponents

	struct ComponentPriorityBlock
	{
		using ComponentContainer = eastl::vector<eastl::shared_ptr<UComponent>>;

		explicit ComponentPriorityBlock(TypeID inComponentTypeID = eastl::numeric_limits<TypeID>::max()) : m_blockComponentTypeID(inComponentTypeID) {}

		TypeID m_blockComponentTypeID;
		ComponentContainer m_blockComponents;
	};

	class ComponentUpdater
	{
	public:
		static const PriorityLevel s_staticPhysicsPriorityLevel;
		
		using ComponentContainer = eastl::multimap<PriorityLevel, ComponentPriorityBlock>;
	public:
		ComponentUpdater();
		
		template <typename ComponentType, typename ...Args>
		eastl::weak_ptr<ComponentType> AddComponent(Args&&... inConstructorArgs);
		
		inline void SetUpdatingFlag(bool inUpdateFlag) { m_isUpdating = inUpdateFlag; }
		inline bool IsUpdating() const { return m_isUpdating; }

		void UpdatePrePhysicsComponents(float inDeltaTime);
		void UpdatePostPhysicsComponents(float inDeltaTime);
	private:
		bool m_isUpdating;
		PriorityLevel m_nextAssignedPriorityLevel;
		
		// Separation between pre and post physics priority blocks to simplify component update and performance
		ComponentContainer m_prePhysicsPriorityBlocks;
		ComponentContainer m_postPhysicsPriorityBlocks;
	};

	template <typename ComponentType, typename ...Args>
	eastl::weak_ptr<ComponentType> ComponentUpdater::AddComponent(Args&&... inConstructorArgs)
	{
		// Need to determine if component type's priority level has been specified as pre or post physics
		TComponentPriorityInfo* componentPriorityInfo = ComponentType::PriorityInfo();
		PriorityLevel componentPriorityLevel = componentPriorityInfo->GetPriorityLevel();
		const TypeID componentTypeID = ComponentType::StaticClass()->GetTypeID();

		// If the component type specified is an un-prioritized component type (using default priority level), we need to change
		// it's priority level to an automatic assigned priority level
		if (componentPriorityLevel == TComponentPriorityInfo::s_defaultPriorityLevel)
		{
			componentPriorityLevel = m_nextAssignedPriorityLevel++;

			componentPriorityInfo->UpdatePriorityLevel(componentPriorityLevel);
		}

		eastl::shared_ptr<ComponentType> newComponentPtr = eastl::make_shared<ComponentType>(eastl::forward<Args>(inConstructorArgs)...);

		ComponentUpdater::ComponentContainer& targetComponentContainer = (componentPriorityLevel < ComponentUpdater::s_staticPhysicsPriorityLevel) ? m_prePhysicsPriorityBlocks : m_postPhysicsPriorityBlocks;

		auto priorityBlockFindIter = targetComponentContainer.equal_range(componentPriorityLevel);

		// Since there can be multiple priority blocks with the same priority, we need to find the one that has the component ID that
		// corresponds with the component type we're trying to add

		while (priorityBlockFindIter.first != priorityBlockFindIter.second)
		{
			if (priorityBlockFindIter.first->second.m_blockComponentTypeID == componentTypeID)
			{
				priorityBlockFindIter.first->second.m_blockComponents.emplace_back(newComponentPtr);
				return newComponentPtr;
			}

			++priorityBlockFindIter.first;
		}

		// If we get here, that means this is the first time we're trying to add a component of this priority level
		auto priorityBlockInsertIter = targetComponentContainer.emplace(componentPriorityLevel);

		priorityBlockInsertIter->second.m_blockComponentTypeID = componentTypeID;
		priorityBlockInsertIter->second.m_blockComponents.emplace_back(newComponentPtr);

		return newComponentPtr;
	}
}
