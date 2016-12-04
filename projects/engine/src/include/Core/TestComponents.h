#pragma once

#include "Core/Component.h"
#include "Core/Entity.h"
#include "Core/ComponentPriorityInfo.h"
#include "Core/PointLightComponent.h"

#define MAD_DEFINE_TEST_COMPONENT(TestComponentName)							\
	DECLARE_LOG_CATEGORY(Log##TestComponentName);											\
																							\
	class C##TestComponentName : public UComponent									\
	{																			\
		MAD_DECLARE_COMPONENT(C##TestComponentName, UComponent)					\
	public:																		\
		explicit C##TestComponentName(OGameWorld* inOwningWorld) : Super(inOwningWorld) {}	\
		virtual void UpdateComponent(float inDeltaTime) override				\
		{																		\
			(void)inDeltaTime;																									\
			LOG(Log##TestComponentName, Log, "Updating " #TestComponentName " for %s #%d\n", GetOwningEntity().GetTypeInfo()->GetTypeName(), GetOwningEntity().GetObjectID());		\
		}																		\
	};																			\



#define MAD_DEFINE_PRIORITIZED_TEST_COMPONENT(TestComponentName, PriorityLevel)				\
	DECLARE_LOG_CATEGORY(Log##TestComponentName);											\
																							\
	class C##TestComponentName : public UComponent												\
	{																						\
		MAD_DECLARE_PRIORITIZED_COMPONENT(C##TestComponentName, UComponent, PriorityLevel)			\
	public:																					\
		explicit C##TestComponentName(OGameWorld* inOwningWorld) : Super(inOwningWorld) {}	\
		virtual void UpdateComponent(float inDeltaTime) override							\
		{																					\
			(void)inDeltaTime;																									\
			LOG(Log##TestComponentName, Log, "Updating " #TestComponentName " for %s #%d\n", GetOwningEntity().GetTypeInfo()->GetTypeName(), GetOwningEntity().GetObjectID());		\
		}																					\
	};																						\


namespace MAD
{
	namespace Test
	{
		MAD_DEFINE_TEST_COMPONENT(SpatialComponent);
		MAD_DEFINE_TEST_COMPONENT(TestComponent1);
		MAD_DEFINE_TEST_COMPONENT(TestComponent2);
		MAD_DEFINE_TEST_COMPONENT(TestComponent3);
		MAD_DEFINE_TEST_COMPONENT(TestComponent4);
		MAD_DEFINE_TEST_COMPONENT(TestComponent5);
		MAD_DEFINE_TEST_COMPONENT(TestComponent6);
		MAD_DEFINE_TEST_COMPONENT(TestComponent7);
		MAD_DEFINE_TEST_COMPONENT(TestComponent8);
		MAD_DEFINE_TEST_COMPONENT(TestComponent9);

		MAD_DEFINE_PRIORITIZED_TEST_COMPONENT(TestComponentA, EPriorityLevelReference::EPriorityLevel_Physics + 1);
		MAD_DEFINE_PRIORITIZED_TEST_COMPONENT(TestComponentB, 2);
		MAD_DEFINE_PRIORITIZED_TEST_COMPONENT(TestComponentC, EPriorityLevelReference::EPriorityLevel_Physics + 2);
		MAD_DEFINE_PRIORITIZED_TEST_COMPONENT(TestComponentE, 4);
		MAD_DEFINE_PRIORITIZED_TEST_COMPONENT(TestComponentF, 5);
		MAD_DEFINE_PRIORITIZED_TEST_COMPONENT(TestComponentG, 6);
		MAD_DEFINE_PRIORITIZED_TEST_COMPONENT(TestComponentH, 7);
		MAD_DEFINE_PRIORITIZED_TEST_COMPONENT(TestComponentI, 8);
		MAD_DEFINE_PRIORITIZED_TEST_COMPONENT(TestComponentJ, 9);

		class CDemoCharacterController : public UComponent
		{
			MAD_DECLARE_COMPONENT(CDemoCharacterController, UComponent)

		public:
			explicit CDemoCharacterController(OGameWorld* inOwningWorld)
				: Super(inOwningWorld)
				, m_lookSpeed(1.0f)
				, m_moveSpeed(300.0f)
			{
				UGameInput::Get().SetMouseMode(EMouseMode::MM_Game);

				auto& demoCharacterScheme = *UGameInput::Get().GetControlScheme("DemoCharacter");

				demoCharacterScheme.BindAxis<CDemoCharacterController, &CDemoCharacterController::MoveForward>("Forward", this);
				demoCharacterScheme.BindAxis<CDemoCharacterController, &CDemoCharacterController::MoveRight>("Horizontal", this);
				demoCharacterScheme.BindAxis<CDemoCharacterController, &CDemoCharacterController::MoveUp>("Vertical", this);
				demoCharacterScheme.BindAxis<CDemoCharacterController, &CDemoCharacterController::LookRight>("LookX", this);
				demoCharacterScheme.BindAxis<CDemoCharacterController, &CDemoCharacterController::LookUp>("LookY", this);

				demoCharacterScheme.BindEvent<CDemoCharacterController, &CDemoCharacterController::OnShoot>("Shoot", EInputEvent::IE_KeyDown, this);
			}

			virtual void OnEvent(EEventTypes inEventType, void* inEventData) override;

		private:
			void MoveRight(float inVal)
			{
				Vector3 right = GetOwningEntity().GetWorldTransform().GetRight();
				GetOwningEntity().SetWorldTranslation(GetOwningEntity().GetWorldTranslation() + right * inVal * gEngine->GetDeltaTime() * m_moveSpeed);
			}

			void MoveForward(float inVal)
			{
				Vector3 forward = GetOwningEntity().GetWorldTransform().GetForward();
				GetOwningEntity().SetWorldTranslation(GetOwningEntity().GetWorldTranslation() + forward * inVal * gEngine->GetDeltaTime() * m_moveSpeed);
			}

			void MoveUp(float inVal)
			{
				GetOwningEntity().SetWorldTranslation(GetOwningEntity().GetWorldTranslation() + Vector3::Up * inVal * gEngine->GetDeltaTime() * m_moveSpeed);
			}

			void LookRight(float inVal)
			{
				auto rot = Quaternion::CreateFromAxisAngle(Vector3::Up, -inVal * gEngine->GetDeltaTime() * m_lookSpeed);
				GetOwningEntity().SetWorldRotation(GetOwningEntity().GetWorldRotation() * rot);
			}

			void LookUp(float inVal)
			{
				Vector3 right = GetOwningEntity().GetWorldTransform().GetRight();
				auto rot = Quaternion::CreateFromAxisAngle(right, -inVal * gEngine->GetDeltaTime() * m_lookSpeed);
				GetOwningEntity().SetWorldRotation(GetOwningEntity().GetWorldRotation() * rot);
			}

			void OnShoot()
			{
				// Send event to spawn point light
				gEngine->GetNetworkManager().SendNetworkEvent(EEventTarget::Server, SHOOT_BULLET, GetOwningEntity(), nullptr, 0);
			}

			float m_lookSpeed;
			float m_moveSpeed;
		};

		class CTimedDeathComponent : public UComponent
		{
			MAD_DECLARE_COMPONENT(CTimedDeathComponent, UComponent)

		public:
			explicit CTimedDeathComponent(OGameWorld* inOwningWorld) : Super(inOwningWorld)
			                                                         , m_lifeTime(-1.0f)
			                                                         , m_lifeTimeOver(FLT_MAX)
			                                                         , m_isServerOnly(false) { }

			virtual void UpdateComponent(float) override
			{
				if (m_isServerOnly && GetNetMode() == ENetMode::Client)
				{
					return;
				}

				float gameTime = gEngine->GetGameTime();

				if (m_lifeTime > 0.0f)
				{
					m_lifeTimeOver = gameTime + m_lifeTime;
					m_lifeTime = -1.0f;
				}

				if (gameTime >= m_lifeTimeOver)
				{
					m_lifeTimeOver = FLT_MAX;

					if (GetNetMode() == ENetMode::Client)
					{
						GetOwningEntity().Destroy();
					}
					else
					{
						gEngine->GetNetworkManager().DestroyNetworkObject(GetOwningEntity());
					}
				}
			}

			void SetLifeTime(float inLifeTime)
			{
				m_lifeTime = inLifeTime;
			}

			void SetServerOnly(bool inIsServerAuthorityOnly)
			{
				m_isServerOnly = inIsServerAuthorityOnly;
			}

		private:
			float m_lifeTime;
			float m_lifeTimeOver;
			bool m_isServerOnly;
		};

		class CPointLightBulletComponent : public UComponent
		{
			MAD_DECLARE_COMPONENT(CPointLightBulletComponent, UComponent)

		public:
			explicit CPointLightBulletComponent(OGameWorld* inOwningWorld) : Super(inOwningWorld)
				, m_nextCycleTime(0)
				, m_colorIndex(0)
				, m_lightColor(Color(1, 1, 1))
			{ }

			virtual void GetReplicatedProperties(eastl::vector<SObjectReplInfo>& inOutReplInfo) const override
			{
				Super::GetReplicatedProperties(inOutReplInfo);

				MAD_ADD_REPLICATION_PROPERTY_CALLBACK(inOutReplInfo, EReplicationType::Always, CPointLightBulletComponent, m_lightColor, OnRep_LightColor);
				MAD_ADD_REPLICATION_PROPERTY(inOutReplInfo, EReplicationType::Always, CPointLightBulletComponent, m_position);
			}

			virtual void UpdateComponent(float inDeltaTime) override
			{
				// Client sets the light's color in the OnRep callback
				if (GetNetMode() == ENetMode::Client)
				{
					GetOwningEntity().SetWorldTranslation(m_position);
					return;
				}

				auto position = GetOwningEntity().GetWorldTranslation();
				position += m_velocity * inDeltaTime;
				m_position = position;
				GetOwningEntity().SetWorldTranslation(position);

				// Server changes the light color every half a second
				float time = gEngine->GetGameTime();
				if (time < m_nextCycleTime)
				{
					return;
				}

				m_nextCycleTime = time + 0.5f;

				// Cycle through Red -> Green -> Blue every half second
				m_lightColor = Color(1.0f * !m_colorIndex, 1.0f * (m_colorIndex & 1), 1.0f * (m_colorIndex & 2));

				// Manually call this for servers since it isn't actually replicated
				OnRep_LightColor();

				m_colorIndex = (m_colorIndex + 1) % 3;
			}

			void SetBulletVelocity(const Vector3& inVelocity) { m_velocity = inVelocity; }

		private:
			Color m_lightColor;
			float m_nextCycleTime;
			int m_colorIndex;
			Vector3 m_velocity;
			Vector3 m_position;
			
			void OnRep_LightColor()
			{
				auto pointLight = GetOwningEntity().GetFirstComponentByType<CPointLightComponent>().lock();
				if (!pointLight) return;

				pointLight->SetColor(m_lightColor);
			}
		};
	}
}
