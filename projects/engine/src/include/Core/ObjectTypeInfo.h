#pragma once

#include <stdint.h>
#include <EASTL/shared_ptr.h>
#include <EASTL/string_hash_map.h>

#include "Core/ComponentPriorityInfo.h"
#include "Misc/Assert.h"

namespace MAD
{
	class OGameWorld;

	using TypeID = uint32_t;

	class TTypeInfo
	{
	public:
		using CreationFunction_t = eastl::shared_ptr<class UObject> (*) (OGameWorld*);
	public:
		static const TTypeInfo* GetTypeInfo(const eastl::string& inTypeName);
		static void DumpTypeInfo();

		TTypeInfo(const TTypeInfo* inParent, const char* inTypeName, CreationFunction_t inCreationFunc);

		inline TypeID GetTypeID() const { return m_typeID; }
		inline const TTypeInfo* GetParent() const { return m_parent; }
		inline const char* GetTypeName() const { return m_typeName; }

		template <typename ObjectType> 
		eastl::shared_ptr<ObjectType> CreateDefaultObject(OGameWorld* inOwningGameWorld) const { return eastl::static_shared_pointer_cast<ObjectType>(m_creationFunction(inOwningGameWorld)); } // Default create an object
	private:
		static TypeID s_currentTypeID;
		static eastl::string_hash_map<const TTypeInfo*> s_typeNameToTypeInfoMap;

		const CreationFunction_t m_creationFunction;
		const char* const m_typeName;
		const TypeID m_typeID;
		const TTypeInfo* const m_parent;
	};

#pragma region Macro Definitions
	// Base object class macro definition
#define MAD_DECLARE_CLASS_COMMON(ClassName)									\
	public:																	\
	virtual const TTypeInfo* GetTypeInfo() const							\
	{																		\
		return ClassName::StaticClass();									\
	}																		\
																			\
	private:																\
	static eastl::shared_ptr<UObject> CreateObject(OGameWorld* inOwningGameWorld)						\
	{																									\
		return eastl::make_shared<ClassName>(inOwningGameWorld);										\
	}																		\
																			\
	friend class TTypeInfo;													\

#define MAD_DECLARE_BASE_CLASS(BaseClass)									\
	MAD_DECLARE_CLASS_COMMON(BaseClass)										\
	public:																	\
		static const TTypeInfo* StaticClass()								\
		{																	\
			static const TTypeInfo s_classTypeInfo(nullptr, #BaseClass, nullptr);	\
			return &s_classTypeInfo;										\
		}																	\
																			\
	private:																\

#define MAD_DECLARE_CLASS(ClassName, ParentClass)							\
	MAD_DECLARE_CLASS_COMMON(ClassName)										\
	public:																	\
		static const TTypeInfo* StaticClass()														\
		{																							\
			static const TTypeInfo s_classTypeInfo(ParentClass::StaticClass(), #ClassName, &ClassName::CreateObject);	\
			return &s_classTypeInfo;																\
		}																							\
																									\
	private:																						\
		using Super = ParentClass;																	\

// Actor specific macro definitions on top of base MAD class macro definitions
#define MAD_DECLARE_ACTOR(ClassName, ParentClass)							\
	MAD_DECLARE_CLASS(ClassName, ParentClass)								\

// Actor component specific macro definitions
#define MAD_DECLARE_COMPONENT_COMMON(ComponentClass, ParentClass, ComponentPriorityLevel)								\
	MAD_DECLARE_CLASS(ComponentClass, ParentClass)																		\
	public:																												\
		static UComponentPriorityInfo* PriorityInfo()																	\
		{																												\
			static UComponentPriorityInfo s_componentPriorityInfo(ComponentPriorityLevel);								\
			return &s_componentPriorityInfo;																			\
		}																												\
																														\
		virtual UComponentPriorityInfo* GetPriorityInfo() override														\
		{																												\
			return ComponentClass::PriorityInfo();																		\
		}																												\
	private:																											\


// Used to define the base UComponent (there is no concept of priority)
#define MAD_DECLARE_BASE_COMPONENT(BaseComponent, ParentClass)																		\
	MAD_DECLARE_CLASS(BaseComponent, ParentClass)																					\
	public:																															\
		virtual UComponentPriorityInfo* GetPriorityInfo() { return nullptr; }														\

#define MAD_DECLARE_COMPONENT(ComponentClass, ParentClass)																	\
	MAD_DECLARE_COMPONENT_COMMON(ComponentClass, ParentClass, EPriorityLevelReference::EPriorityLevel_Default)				\

#define MAD_DECLARE_PRIORITIZED_COMPONENT(ComponentClass, ParentClass, ComponentPriorityLevel)		\
	MAD_DECLARE_COMPONENT_COMMON(ComponentClass, ParentClass, (ComponentPriorityLevel))				\

#pragma endregion

#pragma region RTTI Queries
	template <typename IsAToClass, typename IsAFromClass>
	bool IsA(const IsAFromClass* inObjectPtr)
	{
		// Iterate up the TTypeInfo tree of the IsAFromClass until you find IsAToClass's TTypeInfo or we reach null
		const TTypeInfo* const targetTypeInfo = IsAToClass::StaticClass();
		const TTypeInfo* currentTypeInfo = inObjectPtr->GetTypeInfo();

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

	template <typename IsAFromClass>
	bool IsA(const TTypeInfo& inIsAToClassTypeInfo, const IsAFromClass* inObjectPtr)
	{
		// Iterate up the TTypeInfo tree of the IsAFromClass until you find IsAToClass's TTypeInfo or we reach null
		const TTypeInfo* const targetTypeInfo = &inIsAToClassTypeInfo;
		const TTypeInfo* currentTypeInfo = inObjectPtr->GetTypeInfo();

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

	template <typename IsAToClass>
	bool IsA(const TTypeInfo& inIsAFromClassTypeInfo)
	{
		// Iterate up the TTypeInfo tree of the IsAFromClass until you find IsAToClass's TTypeInfo or we reach null
		const TTypeInfo* const targetTypeInfo = IsAToClass::StaticClass();
		const TTypeInfo* currentTypeInfo = &inIsAFromClassTypeInfo;

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

#pragma region Object Creation
	template <typename ObjectType>
	eastl::shared_ptr<ObjectType> CreateDefaultObject(OGameWorld* inOwningGameWorld)
	{
		const TTypeInfo* objectTypeInfo = ObjectType::StaticClass();
		return objectTypeInfo->CreateDefaultObject<ObjectType>(inOwningGameWorld);
	}

	template <typename ObjectType>
	eastl::shared_ptr<ObjectType> CreateDefaultObject(const TTypeInfo& inTypeInfo, OGameWorld* inOwningGameWorld)
	{
		MAD_ASSERT_DESC(IsA<ObjectType>(inTypeInfo), "Given type info must be a derived class of ObjectType");
		return inTypeInfo.CreateDefaultObject<ObjectType>(inOwningGameWorld);
	}
#pragma endregion
}
