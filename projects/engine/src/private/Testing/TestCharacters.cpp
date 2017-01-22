#include "Testing/TestCharacters.h"
#include <EASTL/weak_ptr.h>
#include "Core/TransformComponent.h"

namespace MAD
{
	namespace Test
	{
		ASpatialCharacter::ASpatialCharacter(OGameWorld* inOwningWorld) : Super_t(inOwningWorld)
		{
			auto spatialComponent = AddComponent<CSpatialComponent>();
			auto childSpatialComponent = AddComponent<CSpatialComponent>();
			auto siblingSpatialComponent = AddComponent<CSpatialComponent>();
			auto grandchildSpatialComponent = AddComponent<CSpatialComponent>();

			//SetRootComponent(spatialComponent);
			childSpatialComponent->SetRelativeTranslation(Vector3(10.0f, 5.0f, 15.0f));
			childSpatialComponent->SetRelativeScale(5.0f);

			siblingSpatialComponent->SetRelativeTranslation(Vector3(-10.0f, 1.0f, 10.0f));
			siblingSpatialComponent->SetRelativeRotation(Quaternion::CreateFromYawPitchRoll(45.0f, 0.0f, 0.0f));

			grandchildSpatialComponent->SetRelativeTranslation(Vector3(25.0f, 0.0f, -55.0f));

			// Attachment of children
			spatialComponent->AttachComponent(childSpatialComponent);
			spatialComponent->AttachComponent(siblingSpatialComponent);
			childSpatialComponent->AttachComponent(grandchildSpatialComponent);

			// Setting world translation only operates correctly after parent-child attachments have been setup
			// Without proper parent-child attachments, setting world space properties are treated as setting relative properties
			//grandchildSpatialComponent.lock()->SetWorldTranslation(Vector3(1.0f, 0.0f, 2.0f)); 
		}

		void RegisterEntityTypes()
		{
			Test::APointLightBullet::StaticClass();
			Test::ADemoCharacter::StaticClass();
		}

	}
}