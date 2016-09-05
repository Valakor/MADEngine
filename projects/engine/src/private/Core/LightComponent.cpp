#include "Core/LightComponent.h"
#include "Core/Entity.h"
#include "Misc/Logging.h"

namespace MAD
{
	DECLARE_LOG_CATEGORY(LogLightComponent);

	void ULightComponent::UpdateComponent(float inDeltaTime)
	{
		(void)inDeltaTime;
		LOG(LogLightComponent, Log, "Updating Light Component for %s #%d\n", GetOwner().GetTypeInfo()->GetTypeName(), GetOwner().GetObjectID());
	}

}