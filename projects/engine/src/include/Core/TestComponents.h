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
			{
				m_lightColor = Color(1, 1, 1);
			}

			virtual void GetReplicatedProperties(eastl::vector<SObjectReplInfo>& inOutReplInfo) const override
			{
				Super::GetReplicatedProperties(inOutReplInfo);

				MAD_ADD_REPLICATION_PROPERTY(inOutReplInfo, EReplicationType::Always, CPointLightBulletComponent, m_lightColor);
			}

			virtual void UpdateComponent(float) override
			{
				auto pointLight = GetOwningEntity().GetFirstComponentByType<CPointLightComponent>().lock();
				if (!pointLight) return;

				// Client simply sets the light's color to the replicated light color
				if (GetNetMode() == ENetMode::Client)
				{
					pointLight->SetColor(m_lightColor);
					return;
				}

				// Server changes the light color every half a second
				float time = gEngine->GetGameTime();
				if (time < m_nextCycleTime)
				{
					return;
				}

				m_nextCycleTime = time + 0.5f;

				if (pointLight)
				{
					// Cycle through Red -> Green -> Blue every half second
					//->SetColor();
					m_lightColor = Color(1.0f * !m_colorIndex, 1.0f * (m_colorIndex & 1), 1.0f * (m_colorIndex & 2));
					pointLight->SetColor(m_lightColor);
				}

				m_colorIndex = (m_colorIndex + 1) % 3;
			}

		private:
			Color m_lightColor;
			float m_nextCycleTime;
			int m_colorIndex;
		};
	}
}