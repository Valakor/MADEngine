#pragma once

#include "Core/Character.h"
#include "Core/TestComponents.h"

#include "Core/PointLightComponent.h"
#include "Core/MeshComponent.h"
#include "Core/CameraComponent.h"
#include "Core/MoveComponent.h"

#include "Networking/Network.h"

namespace MAD
{
	namespace Test
	{
#pragma region Default Entities

		class AMattCharacter : public AEntity
		{
			MAD_DECLARE_ACTOR(AMattCharacter, AEntity)
		public:
			explicit AMattCharacter(OGameWorld* inOwningWorld) : Super_t(inOwningWorld)
			{
				AddComponent<Test::CTestComponentA>();
			}
		};

		class ADerekCharacter : public AEntity
		{
			MAD_DECLARE_ACTOR(ADerekCharacter, AEntity)
		public:
			explicit ADerekCharacter(OGameWorld* inOwningWorld) : Super_t(inOwningWorld)
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

#pragma endregion

#pragma region Transform Hierarchy Root Assignment Testing
		// Testing to make sure that assigning the root component (within spawned entities) works properly if you assigned pre, post, or in the middle

		class APreRootAssignedCharacter : public AEntity
		{
			MAD_DECLARE_ACTOR(APreRootAssignedCharacter, AEntity)
		public:
			explicit APreRootAssignedCharacter(OGameWorld* inOwningWorld) : Super_t(inOwningWorld)
			{
				// Assign root to the first component at the beginning
			}
		};

		class AMidRootAssignedCharacter : public AEntity
		{
			MAD_DECLARE_ACTOR(AMidRootAssignedCharacter, AEntity)
		public:
			explicit AMidRootAssignedCharacter(OGameWorld* inOwningWorld) : Super_t(inOwningWorld)
			{
				// Assign root to the first component in the middle
			}
		};

		class APostRootAssignedCharacter : public AEntity
		{
			MAD_DECLARE_ACTOR(APostRootAssignedCharacter, AEntity)
		public:
			explicit APostRootAssignedCharacter(OGameWorld* inOwningWorld) : Super_t(inOwningWorld)
			{
				// Assign root to the first component at the end
			}
		};

#pragma endregion

#pragma region Demo Entities

		class APointLightBullet : public AEntity
		{
			MAD_DECLARE_ACTOR(APointLightBullet, AEntity)
		public:
			explicit APointLightBullet(OGameWorld* inOwningWorld) : Super_t(inOwningWorld)
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
			explicit ADemoCharacter(OGameWorld* inOwningWorld) : Super_t(inOwningWorld)
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

				auto moveComponent = AddComponent<CMoveComponent>();
				moveComponent->SetTargetComponent(characterMesh.get());
				characterMesh->AttachComponent(moveComponent);
			}

		protected:
			virtual void PostInitializeComponents() override
			{
				Super_t::PostInitializeComponents();

				GetFirstComponentByType<CCameraComponent>().lock()->SetActive(GetNetOwner()->IsLocalPlayer());
			}
		};

		class ANetworkedEntity : public AEntity
		{
			MAD_DECLARE_ACTOR(ANetworkedEntity, AEntity)
		public:
			explicit ANetworkedEntity(OGameWorld* inOwningWorld) : Super_t(inOwningWorld)
			{
			}

			virtual void GetReplicatedProperties(eastl::vector<SObjectReplInfo>& inOutReplInfo) const override
			{
				Super_t::GetReplicatedProperties(inOutReplInfo);

				MAD_ADD_REPLICATION_PROPERTY(inOutReplInfo, EReplicationType::Always, ANetworkedEntity, m_networkedFloat);
				MAD_ADD_REPLICATION_PROPERTY(inOutReplInfo, EReplicationType::Always, ANetworkedEntity, m_networkedUInt);
			}

			float m_networkedFloat;
			uint32_t m_networkedUInt;
		};

#pragma endregion

	}
}
