#include "Core/LightComponent.h"
#include "Core/Entity.h"
#include "Misc/Logging.h"

namespace MAD
{
	DECLARE_LOG_CATEGORY(LogLightComponent);

	CLightComponent::CLightComponent(OGameWorld* inOwningWorld) : Super(inOwningWorld) {}

	void CLightComponent::UpdateComponent(float inDeltaTime)
	{
		 (void)inDeltaTime;
		LOG(LogLightComponent, Log, "Updating Light Component for %s #%d\n", GetOwningEntity().GetTypeInfo()->GetTypeName(), GetOwningEntity().GetObjectID());
	}

}
