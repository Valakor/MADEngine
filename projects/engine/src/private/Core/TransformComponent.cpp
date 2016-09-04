#include "Core/TransformComponent.h"

#include "Core/Entity.h"
#include "Core/GameWorld.h"
#include "Misc/Logging.h"

namespace MAD
{
	MAD_IMPLEMENT_COMPONENT(UTransformComponent)
	DECLARE_LOG_CATEGORY(LogTransformComponent);

	void UTransformComponent::UpdateComponent(float inDeltaTime)
	{
		(void) inDeltaTime;
		LOG(LogTransformComponent, Log, "Updating Transform Component for Entity #%d\n", GetOwner().GetObjectID());
	}
}
