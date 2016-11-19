#pragma once

#include "Core/Component.h"
#include "Core/Entity.h"
#include "Core/ComponentPriorityInfo.h"
#include "LightComponent.h"

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
			                                                         , m_lifeTimeOver(FLT_MAX) { }

			virtual void UpdateComponent(float) override
			{
				float gameTime = gEngine->GetGameTime();

				if (m_lifeTime > 0.0f)
				{
					m_lifeTimeOver = gameTime + m_lifeTime;
					m_lifeTime = -1.0f;
				}

				if (gameTime >= m_lifeTimeOver)
				{
					GetOwningEntity().Destroy();
					m_lifeTimeOver = FLT_MAX;
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
	}
}