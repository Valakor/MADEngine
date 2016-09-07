#pragma once

#include "Core/Component.h"
#include "Core/Entity.h"
#include "Core/ComponentPriorityInfo.h"

#define MAD_DEFINE_TEST_COMPONENT(TestComponentName)							\
	DECLARE_LOG_CATEGORY(Log##TestComponentName);											\
																							\
	class U##TestComponentName : public UComponent									\
	{																			\
		MAD_DECLARE_COMPONENT(U##TestComponentName, UComponent)					\
	public:																		\
		virtual void UpdateComponent(float inDeltaTime) override				\
		{																		\
			(void)inDeltaTime;																									\
			LOG(Log##TestComponentName, Log, "Updating " #TestComponentName " for %s #%d\n", GetOwner().GetTypeInfo()->GetTypeName(), GetOwner().GetObjectID());		\
		}																		\
	};																			\



#define MAD_DEFINE_PRIORITIZED_TEST_COMPONENT(TestComponentName, PriorityLevel)				\
	DECLARE_LOG_CATEGORY(Log##TestComponentName);											\
																							\
	class U##TestComponentName : public UComponent												\
	{																						\
		MAD_DECLARE_PRIORITIZED_COMPONENT(U##TestComponentName, UComponent, PriorityLevel)			\
	public:																					\
		virtual void UpdateComponent(float inDeltaTime) override							\
		{																					\
			(void)inDeltaTime;																									\
			LOG(Log##TestComponentName, Log, "Updating " #TestComponentName " for %s #%d\n", GetOwner().GetTypeInfo()->GetTypeName(), GetOwner().GetObjectID());		\
		}																					\
	};																						\


namespace MAD
{
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
}