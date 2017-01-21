#include "Testing/TestComponents.h"
#include "Testing/TestCharacters.h"

namespace MAD
{
	namespace Test
	{
		void RegisterComponentTypes()
		{
			CTimedDeathComponent::StaticClass();
			CPointLightBulletComponent::StaticClass();
			CDemoCharacterController::StaticClass();
			CSinMoveComponent::StaticClass();
			CCircularMoveComponent::StaticClass();
		}

		void CDemoCharacterController::OnEvent(EEventTypes inEventType, void* inEventData)
		{
			Super_t::OnEvent(inEventType, inEventData);

			if (inEventType == SHOOT_BULLET)
			{
				MAD_ASSERT_DESC(GetNetMode() != ENetMode::Client, "Error: Trying to spawn point light on the client!\n");

				auto bullet = gEngine->GetNetworkManager().SpawnNetworkEntity<APointLightBullet>(this, GetOwningWorld(), GetOwningEntity().GetOwningWorldLayer().GetLayerName());
				if (bullet)
				{
					auto bulletLightComponent = bullet->GetFirstComponentByType<CPointLightBulletComponent>().lock();
					if (bulletLightComponent)
					{
						Vector3 bulletInitVelocity = GetOwningEntity().GetWorldTransform().GetForward() * 170.0f;
						bulletLightComponent->SetBulletVelocity(bulletInitVelocity);
						bullet->SetWorldTranslation(GetOwningEntity().GetWorldTranslation());
					}
				}
			}
		}
	}
}
