#include "Core/MeshComponent.h"
#include "Core/Entity.h"
#include "Misc/Logging.h"

namespace MAD
{
	DECLARE_LOG_CATEGORY(LogTransformComponent);

	CMeshComponent::CMeshComponent(OGameWorld* inOwningWorld)
		: Super(inOwningWorld) {}

	void CMeshComponent::UpdateComponent(float inDeltaTime)
	{
		(void)inDeltaTime;

		LOG(LogTransformComponent, Log, "Updating Mesh Component for %s #%d\n", GetOwner().GetTypeInfo()->GetTypeName(), GetOwner().GetObjectID());
	}
}
