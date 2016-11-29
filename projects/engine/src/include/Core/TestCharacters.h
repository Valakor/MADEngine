#pragma once

#include "Core/Character.h"
#include "Core/TestComponents.h"

#include "Core/PointLightComponent.h"
#include "Core/MeshComponent.h"

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
				mesh->LoadFrom("engine\\meshes\\primitives\\sphere.obj");
				mesh->SetRelativeScale(10.0f);
				mesh->SetVisible(true);
				SetRootComponent(mesh);

				auto pointLight = AddComponent<CPointLightComponent>();
				pointLight->SetEnabled(true);
				mesh->AttachComponent(pointLight);

				auto pointLightBullet = AddComponent<CPointLightBulletComponent>();
				mesh->AttachComponent(pointLightBullet);

				auto destroyMe = AddComponent<CTimedDeathComponent>();
				destroyMe->SetLifeTime(3.5f);
				destroyMe->SetServerOnly(true);
				mesh->AttachComponent(destroyMe);
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
