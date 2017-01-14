#pragma once

#include "Core/Component.h"
#include "Core/Entity.h"
#include "Core/ComponentPriorityInfo.h"
#include "Core/PointLightComponent.h"
#include "Core/MoveComponent.h"
#include "Core/GameWorldLoader.h"

#define MAD_DEFINE_TEST_COMPONENT(TestComponentName)							\
	DECLARE_LOG_CATEGORY(Log##TestComponentName);											\
																							\
	class C##TestComponentName : public UComponent									\
	{																			\
		MAD_DECLARE_COMPONENT(C##TestComponentName, UComponent)					\
	public:																		\
		explicit C##TestComponentName(OGameWorld* inOwningWorld) : Super_t(inOwningWorld) {}	\
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
		explicit C##TestComponentName(OGameWorld* inOwningWorld) : Super_t(inOwningWorld) {}	\
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
				: Super_t(inOwningWorld)
				, m_lookSpeed(1.0f)
				, m_moveSpeed(300.0f)
			{ }

			virtual void OnBeginPlay() override
			{
				Super_t::OnBeginPlay();

				if (GetNetOwner()->IsLocalPlayer())
				{
					UGameInput::Get().SetMouseMode(EMouseMode::MM_Game);

					auto& demoCharacterScheme = *UGameInput::Get().GetControlScheme("DemoCharacter");

					demoCharacterScheme.BindAxis<CDemoCharacterController, &CDemoCharacterController::MoveForward>("Forward", this);
					demoCharacterScheme.BindAxis<CDemoCharacterController, &CDemoCharacterController::MoveRight>("Horizontal", this);
					demoCharacterScheme.BindAxis<CDemoCharacterController, &CDemoCharacterController::MoveUp>("Vertical", this);
					demoCharacterScheme.BindAxis<CDemoCharacterController, &CDemoCharacterController::LookRight>("LookX", this);
					demoCharacterScheme.BindAxis<CDemoCharacterController, &CDemoCharacterController::LookUp>("LookY", this);

					demoCharacterScheme.BindEvent<CDemoCharacterController, &CDemoCharacterController::OnShoot>("Shoot", EInputEvent::IE_KeyDown, this);
					demoCharacterScheme.BindEvent<CDemoCharacterController, &CDemoCharacterController::ToggleMouseLock>("MouseMode", EInputEvent::IE_KeyDown, this);
				}
			}

			virtual void OnEvent(EEventTypes inEventType, void* inEventData) override;

		private:
			ULinearTransform m_transform;

			void Move(const Vector3& inDelta)
			{
				if (inDelta == Vector3::Zero)
				{
					return;
				}

				if (UGameInput::Get().GetMouseMode() == EMouseMode::MM_Game)
				{
					m_transform.SetTranslation(m_transform.GetTranslation() + inDelta);

					auto owningMoveComp = GetOwningEntity().GetFirstComponentByType<CMoveComponent>();
					if (!owningMoveComp.expired())
					{
						owningMoveComp.lock()->AddDeltaPosition(inDelta);
					}
				}
			}

			void Look(const Quaternion& inDelta)
			{
				if (inDelta == Quaternion::Identity)
				{
					return;
				}

				if (UGameInput::Get().GetMouseMode() == EMouseMode::MM_Game)
				{
					m_transform.SetRotation(m_transform.GetRotation() * inDelta);

					auto owningMoveComp = GetOwningEntity().GetFirstComponentByType<CMoveComponent>();
					if (!owningMoveComp.expired())
					{
						owningMoveComp.lock()->AddDeltaRotation(inDelta);
					}
				}
			}

			void MoveRight(float inVal)
			{
				Vector3 right = m_transform.GetRight();
				Move(right * inVal * gEngine->GetDeltaTime() * m_moveSpeed);
			}

			void MoveForward(float inVal)
			{
				Vector3 forward = m_transform.GetForward();
				Move(forward * inVal * gEngine->GetDeltaTime() * m_moveSpeed);
			}

			void MoveUp(float inVal)
			{
				Move(Vector3::Up * inVal * gEngine->GetDeltaTime() * m_moveSpeed);
			}

			void LookRight(float inVal)
			{
				Look(Quaternion::CreateFromAxisAngle(Vector3::Up, -inVal * gEngine->GetDeltaTime() * m_lookSpeed));
			}

			void LookUp(float inVal)
			{
				Vector3 right = m_transform.GetRight();
				Look(Quaternion::CreateFromAxisAngle(right, -inVal * gEngine->GetDeltaTime() * m_lookSpeed));
			}

			void OnShoot()
			{
				// Send event to spawn point light
				gEngine->GetNetworkManager().SendNetworkEvent(EEventTarget::Server, SHOOT_BULLET, GetOwningEntity(), nullptr, 0);
			}

			void ToggleMouseLock() const
			{
				static EMouseMode s_mouseMode = UGameInput::Get().GetMouseMode();

				if (s_mouseMode == EMouseMode::MM_Game)
				{
					UGameInput::Get().SetMouseMode(EMouseMode::MM_UI);
				}
				else
				{
					UGameInput::Get().SetMouseMode(EMouseMode::MM_Game);
				}

				s_mouseMode = UGameInput::Get().GetMouseMode();
			}

			float m_lookSpeed;
			float m_moveSpeed;
		};

		class CTimedDeathComponent : public UComponent
		{
			MAD_DECLARE_COMPONENT(CTimedDeathComponent, UComponent)

		public:
			explicit CTimedDeathComponent(OGameWorld* inOwningWorld) : Super_t(inOwningWorld)
			                                                         , m_lifeTime(-1.0f)
			                                                         , m_lifeTimeOver(FLT_MAX) { }

			virtual void UpdateComponent(float) override
			{
				if (GetNetRole() != ENetRole::Authority)
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
					gEngine->GetNetworkManager().DestroyNetworkObject(GetOwningEntity());
				}
			}

			void SetLifeTime(float inLifeTime)
			{
				m_lifeTime = inLifeTime;
			}

		private:
			float m_lifeTime;
			float m_lifeTimeOver;
		};

		class CPointLightBulletComponent : public UComponent
		{
			MAD_DECLARE_COMPONENT(CPointLightBulletComponent, UComponent)

		public:
			explicit CPointLightBulletComponent(OGameWorld* inOwningWorld) : Super_t(inOwningWorld)
				, m_nextCycleTime(0)
				, m_colorIndex(0)
				, m_lightColor(Color(1, 1, 1))
			{ }

			virtual void OnBeginPlay() override
			{
				if (GetNetRole() == ENetRole::Authority)
				{
					UpdateColor();
					m_position = GetOwningEntity().GetWorldTranslation();
				}
			}

			virtual void GetReplicatedProperties(eastl::vector<SObjectReplInfo>& inOutReplInfo) const override
			{
				Super_t::GetReplicatedProperties(inOutReplInfo);

				MAD_ADD_REPLICATION_PROPERTY_CALLBACK(inOutReplInfo, EReplicationType::Always, CPointLightBulletComponent, m_lightColor, OnRep_LightColor);
				MAD_ADD_REPLICATION_PROPERTY(inOutReplInfo, EReplicationType::Always, CPointLightBulletComponent, m_position);
			}

			virtual void UpdateComponent(float inDeltaTime) override
			{
				// Client sets the light's color in the OnRep callback
				if (GetNetRole() == ENetRole::Simulated_Proxy || GetNetRole() == ENetRole::Authority_Proxy)
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

				UpdateColor();
			}

			void SetBulletVelocity(const Vector3& inVelocity) { m_velocity = inVelocity; }

		private:
			Color m_lightColor;
			float m_nextCycleTime;
			int m_colorIndex;
			Vector3 m_velocity;
			Vector3 m_position;

			void UpdateColor()
			{
				// Cycle through Red -> Green -> Blue every half second
				m_lightColor = Color(1.0f * !m_colorIndex, 1.0f * (m_colorIndex & 1), 1.0f * (m_colorIndex & 2));

				// Manually call this for servers since it isn't actually replicated
				OnRep_LightColor();

				m_colorIndex = (m_colorIndex + 1) % 3;
			}
			
			void OnRep_LightColor()
			{
				auto pointLight = GetOwningEntity().GetFirstComponentByType<CPointLightComponent>().lock();
				if (!pointLight) return;

				pointLight->SetColor(m_lightColor);
			}

		};

		class CSinMoveComponent : public UComponent
		{
			MAD_DECLARE_COMPONENT(CSinMoveComponent, UComponent)
		public:
			explicit CSinMoveComponent(OGameWorld* inOwningWorld) : Super_t(inOwningWorld)
			                                                      , m_moveSpeed(1.0f)
			                                                      , m_distance(1.0f) { }

			virtual void UpdateComponent(float) override
			{
				/*auto root = GetOwningEntity().GetRootComponent();
				
				float gameTime = gEngine->GetGameTime();
				Vector3 offset = Vector3::Up * sinf(gameTime * m_moveSpeed) * m_distance;

				root->SetWorldTranslation(m_initialPosition + offset);*/
			}

			virtual void Load(const UGameWorldLoader& inLoader) override
			{
				inLoader.GetFloat("speed", m_moveSpeed);
				inLoader.GetFloat("distance", m_distance);
			}

			virtual void OnBeginPlay() override
			{
				m_initialPosition = GetOwningEntity().GetRootComponent()->GetWorldTranslation();
			}

		private:
			float m_moveSpeed;
			float m_distance;
			Vector3 m_initialPosition;
		};
	}
}
