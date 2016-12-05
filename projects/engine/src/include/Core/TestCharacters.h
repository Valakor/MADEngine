#pragma once

#include "Core/Character.h"
#include "Core/TestComponents.h"

#include "Core/PointLightComponent.h"
#include "Core/MeshComponent.h"
#include "Core/CameraComponent.h"

#include "Networking/Network.h"

namespace MAD
{
	namespace Test
	{
		class AMattCharacter : public AEntity
		{
			MAD_DECLARE_ACTOR(AMattCharacter, AEntity)
		public:
			explicit AMattCharacter(OGameWorld* inOwningWorld) : Super(inOwningWorld)
			{
				AddComponent<Test::CTestComponentA>();
			}
		};

		class ADerekCharacter : public AEntity
		{
			MAD_DECLARE_ACTOR(ADerekCharacter, AEntity)
		public:
			explicit ADerekCharacter(OGameWorld* inOwningWorld) : Super(inOwningWorld)
			{
				AddComponent<Test::CTestComponent5>();
			}
		};

		class ASpatialCharacter : public AEntity
		{
			MAD_DECLARE_ACTOR(ASpatialCharacter, AEntity)
		public:
			explicit ASpatialCharacter(OGameWorld* inOwningWorld);
		};

		class APointLightBullet : public AEntity
		{
			MAD_DECLARE_ACTOR(APointLightBullet, AEntity)
		public:
			explicit APointLightBullet(OGameWorld* inOwningWorld) : Super(inOwningWorld)
			{
				auto mesh = AddComponent<CMeshComponent>();
				mesh->LoadFrom("meshes\\bullet.obj");
				mesh->SetRelativeScale(20.0f);
				mesh->SetVisible(true);
				SetRootComponent(mesh);

				auto pointLight = AddComponent<CPointLightComponent>();
				pointLight->SetEnabled(true);
				mesh->AttachComponent(pointLight);

				auto pointLightBullet = AddComponent<CPointLightBulletComponent>();
				mesh->AttachComponent(pointLightBullet);

				auto destroyMe = AddComponent<CTimedDeathComponent>();
				destroyMe->SetLifeTime(5.0f);
				mesh->AttachComponent(destroyMe);
			}
		};

		class ADemoCharacter : public AEntity
		{
			MAD_DECLARE_ACTOR(ADemoCharacter, AEntity)
		public:
			explicit ADemoCharacter(OGameWorld* inOwningWorld) : Super(inOwningWorld)
			{
				auto characterMesh = AddComponent<CMeshComponent>();
				characterMesh->LoadFrom("engine\\meshes\\primitives\\cube.obj");
				characterMesh->SetVisible(true);
				characterMesh->SetRelativeScale(50.0f);
				SetRootComponent(characterMesh);

				auto cameraComponent = AddComponent<CCameraComponent>();
				cameraComponent->SetRelativeTranslation(Vector3(0.0f, 75.f, 225.0f));
				characterMesh->AttachComponent(cameraComponent);

				auto controller = AddComponent<CDemoCharacterController>();
				characterMesh->AttachComponent(controller);
			}

		protected:
			virtual void PostInitializeComponents() override
			{
				Super::PostInitializeComponents();

				GetFirstComponentByType<CCameraComponent>().lock()->SetActive(GetNetOwner()->IsLocalPlayer());
			}
		};

		class ANetworkedEntity : public AEntity
		{
			MAD_DECLARE_ACTOR(ANetworkedEntity, AEntity)
		public:
			explicit ANetworkedEntity(OGameWorld* inOwningWorld) : Super(inOwningWorld)
			{
			}

			virtual void GetReplicatedProperties(eastl::vector<SObjectReplInfo>& inOutReplInfo) const override
			{
				Super::GetReplicatedProperties(inOutReplInfo);

				MAD_ADD_REPLICATION_PROPERTY(inOutReplInfo, EReplicationType::Always, ANetworkedEntity, m_networkedFloat);
				MAD_ADD_REPLICATION_PROPERTY(inOutReplInfo, EReplicationType::Always, ANetworkedEntity, m_networkedUInt);
			}

			float m_networkedFloat;
			uint32_t m_networkedUInt;
		};
	}
}
