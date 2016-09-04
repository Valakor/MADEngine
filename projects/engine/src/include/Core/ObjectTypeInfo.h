#pragma once

#include <stdint.h>

#include "Core/ComponentPriorityInfo.h"

namespace MAD
{
	using TypeID = uint32_t;

	class TTypeInfo
	{
	public:
		static TypeID s_currentTypeID;
		using CreationFunction_t = class UObject* (*) ();
	public:
		explicit TTypeInfo(const TTypeInfo* inParent, CreationFunction_t inCreationFunc);

		inline TypeID GetTypeID() const { return m_typeID; }
		inline const TTypeInfo* GetParent() const { return m_parent; }

	private:
		const CreationFunction_t m_creationFunction;
		const TypeID m_typeID;
		const TTypeInfo* m_parent;
	};

#pragma region Macro Definitions
	// Base object class macro definition
#define MAD_DECLARE_CLASS_COMMON(ClassName)									\
	private:																\
	static UObject* CreateObject()											\
	{																		\
		return new ClassName();												\
	}																		\
																			\
	friend class TTypeInfo;													\

#define MAD_DECLARE_BASE_CLASS(BaseClass)									\
	MAD_DECLARE_CLASS_COMMON(BaseClass)										\
	public:																	\
		static const TTypeInfo* StaticClass()								\
		{																	\
			static TTypeInfo s_classTypeInfo(nullptr, &BaseClass::CreateObject);						\
			return &s_classTypeInfo;										\
		}																	\
	private:																\

#define MAD_DECLARE_CLASS(ClassName, ParentClass)							\
	MAD_DECLARE_CLASS_COMMON(ClassName)										\
	public:																	\
		static const TTypeInfo* StaticClass()														\
		{																							\
			static TTypeInfo s_classTypeInfo(ParentClass::StaticClass(), &ClassName::CreateObject);	\
			return &s_classTypeInfo;										\
		}																	\
	private:																\
		using Super = ParentClass;											\

// Actor specific macro definitions on top of base MAD class macro definitions
#define MAD_DECLARE_ACTOR(ClassName, ParentClass)							\
	MAD_DECLARE_CLASS(ClassName, ParentClass)								\

// Actor component specific macro definitions
#define MAD_DECLARE_COMPONENT_COMMON(ComponentClass, ParentClass, ComponentPriorityLevel)								\
	MAD_DECLARE_CLASS(ComponentClass, ParentClass)																		\
	public:																												\
		static eastl::weak_ptr<ComponentClass> CreateInstance(class AEntity& inComponentOwner);							\
																														\
		static TComponentPriorityInfo* PriorityInfo()																	\
		{																												\
			static TComponentPriorityInfo s_componentPriorityInfo(ComponentPriorityLevel);								\
			return &s_componentPriorityInfo;																			\
		}																												\
	private:																											\

#define MAD_DECLARE_COMPONENT(ComponentClass, ParentClass)																	\
	MAD_DECLARE_COMPONENT_COMMON(ComponentClass, ParentClass, TComponentPriorityInfo::s_defaultPriorityLevel)				\

#define MAD_DECLARE_PRIORITIZED_COMPONENT(ComponentClass, ParentClass, ComponentPriorityLevel)			\
	MAD_DECLARE_COMPONENT_COMMON(ComponentClass, ParentClass, (ComponentPriorityLevel) + 1)				\

#define MAD_IMPLEMENT_COMPONENT(ComponentClass)																\
	eastl::weak_ptr<ComponentClass> CreateInstance(AEntity& inComponentOwner)								\
	{																										\
		UGameWorld& owningGameWorld = inComponentOwner.GetOwningWorldLayer().GetOwningWorld();				\
																											\
		return owningGameWorld.GetComponentUpdater().AddComponent<ComponentClass>(inComponentOwner);		\
	}																										\

#pragma endregion

#pragma region RTTI Queries
	template <typename IsAToClass, typename IsAFromClass>
	bool IsA(const IsAFromClass* inObjectPtr)
	{
		(void)inObjectPtr;

		// Iterate up the TTypeInfo tree of the IsAFromClass until you find IsAToClass's TTypeInfo or we reach null
		const TTypeInfo* const targetTypeInfo = IsAToClass::StaticClass();
		const TTypeInfo* currentTypeInfo = IsAFromClass::StaticClass();

		while (currentTypeInfo)
		{
			if (currentTypeInfo == targetTypeInfo)
			{
				return true;
			}

			currentTypeInfo = currentTypeInfo->GetParent();
		}

		return false;
	}

	template <typename CastToClass, typename CastFromClass>
	const CastToClass* Cast(const CastFromClass* inInitialObjectPtr)
	{
		if (IsA<CastToClass>(inInitialObjectPtr))
		{
			return reinterpret_cast<const CastToClass*>(inInitialObjectPtr);
		}
		else
		{
			return nullptr;
		}
	}

	template <typename CastToClass, typename CastFromClass>
	CastToClass* Cast(CastFromClass* inInitialObjectPtr)
	{
		if (IsA<CastToClass>(inInitialObjectPtr))
		{
			return reinterpret_cast<CastToClass*>(inInitialObjectPtr);
		}
		else
		{
			return nullptr;
		}
	}
#pragma endregion
}
