#include "Core/TestCharacters.h"
#include <EASTL/weak_ptr.h>
#include "Core/TransformComponent.h"

namespace MAD
{
	namespace Test
	{
		ASpatialCharacter::ASpatialCharacter(OGameWorld* inOwningWorld) : Super(inOwningWorld)
		{
			auto spatialComponent = AddComponent<CSpatialComponent>();
			auto childSpatialComponent = AddComponent<CSpatialComponent>();
			auto siblingSpatialComponent = AddComponent<CSpatialComponent>();
			auto grandchildSpatialComponent = AddComponent<CSpatialComponent>();

			SetRootComponent(spatialComponent);

			childSpatialComponent.lock()->SetRelativeTranslation(Vector3(10.0f, 5.0f, 15.0f));
			childSpatialComponent.lock()->SetRelativeScale(5.0f);

			siblingSpatialComponent.lock()->SetRelativeTranslation(Vector3(-10.0f, 1.0f, 10.0f));
			siblingSpatialComponent.lock()->SetRelativeRotation(Quaternion::CreateFromYawPitchRoll(45.0f, 0.0f, 0.0f));

			grandchildSpatialComponent.lock()->SetRelativeTranslation(Vector3(25.0f, 0.0f, -55.0f));

			// Attachment of children
			spatialComponent.lock()->AttachComponent(childSpatialComponent.lock()); // Ew
			spatialComponent.lock()->AttachComponent(siblingSpatialComponent.lock()); // Ew
			childSpatialComponent.lock()->AttachComponent(grandchildSpatialComponent.lock()); // Ew

			// Setting world translation only operates correctly after parent-child attachments have been setup
			// Without proper parent-child attachments, setting world space properties are treated as setting relative properties
			//grandchildSpatialComponent.lock()->SetWorldTranslation(Vector3(1.0f, 0.0f, 2.0f)); 
		}
	}
}