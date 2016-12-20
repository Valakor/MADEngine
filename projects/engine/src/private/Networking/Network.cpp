#include "Networking/Network.h"
#include "Core/Component.h"
#include "Core/Entity.h"

namespace MAD
{
	int32_t DetermineComponentIndex(const UComponent* inTargetComponent)
	{
		if (!inTargetComponent)
		{
			return SObjectReplInfo::InvalidIndex;
		}

		const AEntity& owningEntity = inTargetComponent->GetOwningEntity();

		ConstComponentContainer_t entityComponents;

		owningEntity.GetEntityComponents(entityComponents);

		const size_t numEntityComps = entityComponents.size();

		for (int32_t i = 0; i < numEntityComps; ++i)
		{
			if (!entityComponents[i].expired() && entityComponents[i].lock().get() == inTargetComponent)
			{
				return i;
			}
		}

		return SObjectReplInfo::InvalidIndex;
	}

}