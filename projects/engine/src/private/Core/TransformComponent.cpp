#include "Core/TransformComponent.h"

#include "Core/Entity.h"
#include "Core/GameWorld.h"
#include "Misc/Logging.h"

namespace MAD
{
	DECLARE_LOG_CATEGORY(LogTransformComponent);

	CTransformComponent::CTransformComponent(OGameWorld* inOwningWorld)
		: Super_t(inOwningWorld) {}

	void CTransformComponent::UpdateComponent(float inDeltaTime)
	{
		if (m_updateCount < 2)
		{
			(void) inDeltaTime;
			LOG(LogTransformComponent, Log, "Updating Transform Component for %s #%d\n", GetOwningEntity().GetTypeInfo()->GetTypeName(), GetOwningEntity().GetObjectID());
		}
		else
		{
			GetOwningEntity().Destroy();
			LOG(LogTransformComponent, Log, "Mark destruction for %s #%d from Transform Component\n", GetOwningEntity().GetTypeInfo()->GetTypeName(), GetOwningEntity().GetObjectID());
		}

		++m_updateCount;

	}
}
