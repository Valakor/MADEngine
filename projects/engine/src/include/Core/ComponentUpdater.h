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
		using ComponentContainer = eastl::multimap<PriorityLevel, ComponentPriorityBlock>;

		friend class UGameWorld;
	public:
		ComponentUpdater();

		void RemoveComponent(eastl::shared_ptr<UComponent> inTargetComponent);

		inline void SetUpdatingFlag(bool inUpdateFlag) { m_isUpdating = inUpdateFlag; }
		inline bool IsUpdating() const { return m_isUpdating; }

		void UpdatePrePhysicsComponents(float inDeltaTime);
		void UpdatePostPhysicsComponents(float inDeltaTime);
	private:
		void RegisterComponent(eastl::shared_ptr<UComponent> inNewComponentPtr);
	private:
		bool m_isUpdating;
		ComponentContainer m_componentPriorityBlocks;
	};
}
