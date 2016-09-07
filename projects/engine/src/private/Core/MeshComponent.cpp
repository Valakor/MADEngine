#include "Core/MeshComponent.h"
#include "Core/Entity.h"
#include "Misc/Logging.h"

namespace MAD
{
	DECLARE_LOG_CATEGORY(LogTransformComponent);

	void UMeshComponent::UpdateComponent(float inDeltaTime)
	{
		(void)inDeltaTime;

		LOG(LogTransformComponent, Log, "Updating Mesh Component for %s #%d\n", GetOwner().GetTypeInfo()->GetTypeName(), GetOwner().GetObjectID());
	}
}
