#include "Core/TestComponents.h"
#include "Core/TestCharacters.h"

namespace MAD
{
	namespace Test
	{
		void CDemoCharacterController::OnEvent(EEventTypes inEventType, void* inEventData)
		{
			Super::OnEvent(inEventType, inEventData);

			if (inEventType == SHOOT_BULLET)
			{
				MAD_ASSERT_DESC(GetNetMode() != ENetMode::Client, "Error: Trying to spawn point light on the client!\n");

				auto bullet = gEngine->GetNetworkManager().SpawnNetworkEntity<APointLightBullet>(GetOwningWorld(), GetOwningEntity().GetOwningWorldLayer().GetLayerName());
				if (bullet)
				{
					auto bulletLightComponent = bullet->GetFirstComponentByType<CPointLightBulletComponent>().lock();
					if (bulletLightComponent)
					{
						Vector3 bulletInitVelocity = GetOwningEntity().GetWorldTransform().GetForward() * 700.0f;
						bulletLightComponent->SetBulletVelocity(bulletInitVelocity);
						bullet->SetWorldTranslation(GetOwningEntity().GetWorldTranslation());
					}
				}
			}
		}
	}
}
