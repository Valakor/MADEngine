#include "Core/TransformComponent.h"

#include "Core/Entity.h"
#include "Misc/Logging.h"

namespace MAD
{
	DECLARE_LOG_CATEGORY(LogTransformComponent);

	UTransformComponent::UTransformComponent(AEntity& inCompOwner) : Super(inCompOwner) {}

	void UTransformComponent::UpdateComponent(float inDeltaTime)
	{
		(void) inDeltaTime;
		LOG(LogTransformComponent, Log, "Updating Transform Component for Entity #%d\n", GetOwner().GetObjectID());
	}
}
