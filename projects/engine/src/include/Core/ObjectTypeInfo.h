#pragma once

#include <stdint.h>
#include <EASTL/shared_ptr.h>

#include "Core/ComponentPriorityInfo.h"

namespace MAD
{
	using TypeID = uint32_t;

	class TTypeInfo
	{
	public:
		static TypeID s_currentTypeID;
		using CreationFunction_t = eastl::shared_ptr<class UObject> (*) ();
	public:
		TTypeInfo(const TTypeInfo* inParent, const char* inTypeName, CreationFunction_t inCreationFunc);

		inline TypeID GetTypeID() const { return m_typeID; }
		inline const TTypeInfo* GetParent() const { return m_parent; }
		inline const char* GetTypeName() const { return m_typeName; }

		template <typename ObjectType> 
		eastl::shared_ptr<ObjectType> CreateDefaultObject() const { return eastl::static_shared_pointer_cast<ObjectType>(m_creationFunction()); } // Default create an object
	private:
		const CreationFunction_t m_creationFunction;
		const char* const m_typeName;
		const TypeID m_typeID;
		const TTypeInfo* const m_parent;
	};

#pragma region Macro Definitions
	// Base object class macro definition
#define MAD_DECLARE_CLASS_COMMON(ClassName)									\
	private:																\
	static eastl::shared_ptr<UObject> CreateObject()						\
	{																		\
		return eastl::shared_ptr<ClassName>(new ClassName());				\
	}																		\
																			\
	friend class TTypeInfo;													\

#define MAD_DECLARE_BASE_CLASS(BaseClass)									\
	public:																	\
		static const TTypeInfo* StaticClass()								\
		{																	\
			static TTypeInfo s_classTypeInfo(nullptr, #BaseClass, nullptr);				\
			return &s_classTypeInfo;											\
		}																	\
																			\
		virtual const TTypeInfo* GetTypeInfo()								\
		{																	\
			return BaseClass::StaticClass();								\
		}																	\
	private:																\

#define MAD_DECLARE_CLASS(ClassName, ParentClass)							\
	MAD_DECLARE_CLASS_COMMON(ClassName)										\
	public:																	\
		static const TTypeInfo* StaticClass()														\
		{																							\
			static TTypeInfo s_classTypeInfo(ParentClass::StaticClass(), #ClassName, &ClassName::CreateObject);	\
			return &s_classTypeInfo;																	\
		}																							\
																									\
		virtual const TTypeInfo* GetTypeInfo()														\
		{																							\
			return ClassName::StaticClass();														\
		}																							\
	private:																						\
		using Super = ParentClass;																	\

// Actor specific macro definitions on top of base MAD class macro definitions
#define MAD_DECLARE_ACTOR(ClassName, ParentClass)							\
	MAD_DECLARE_CLASS(ClassName, ParentClass)								\

// Actor component specific macro definitions
#define MAD_DECLARE_COMPONENT_COMMON(ComponentClass, ParentClass, ComponentPriorityLevel)								\
	MAD_DECLARE_CLASS(ComponentClass, ParentClass)																		\
	public:																												\
		static TComponentPriorityInfo* PriorityInfo()																	\
		{																												\
			static TComponentPriorityInfo s_componentPriorityInfo(ComponentPriorityLevel);								\
			return &s_componentPriorityInfo;																				\
		}																												\
																														\
		virtual TComponentPriorityInfo* GetPriorityInfo() override														\
		{																												\
			return ComponentClass::PriorityInfo();																		\
		}																												\
	private:																											\


// Used to define the base UComponent (there is no concept of priority)
#define MAD_DECLARE_BASE_COMPONENT(BaseComponent, ParentClass)																		\
	MAD_DECLARE_CLASS(BaseComponent, ParentClass)																					\
	public:																															\
		virtual TComponentPriorityInfo* GetPriorityInfo() { return nullptr; }														\

#define MAD_DECLARE_COMPONENT(ComponentClass, ParentClass)																	\
	MAD_DECLARE_COMPONENT_COMMON(ComponentClass, ParentClass, EPriorityLevelReference::EPriorityLevel_Default)				\

#define MAD_DECLARE_PRIORITIZED_COMPONENT(ComponentClass, ParentClass, ComponentPriorityLevel)		\
	MAD_DECLARE_COMPONENT_COMMON(ComponentClass, ParentClass, (ComponentPriorityLevel))				\

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
