#pragma once

#include "Misc\ErrorHandling.h"

#include <memory>

namespace MAD
{
	class TTypeInfo
	{
	public:
		explicit TTypeInfo(const TTypeInfo* inParent = nullptr);

		inline const TTypeInfo* GetParent() const { return m_parent; }
	private:
		const TTypeInfo* m_parent;
	};

#pragma region Macro Definitions
	// Base object class macro definition
#define MAD_DECLARE_CLASS(ClassName)										\
	public:																	\
		static const TTypeInfo* StaticClass() { return &s_classTypeInfo; }	\
	private:																\
		static const TTypeInfo s_classTypeInfo;								\
	private:																\

#define MAD_IMPLEMENT_CLASS(ChildClass, ParentClass)							\
	using Super = ParentClass;													\
	const TTypeInfo ChildClass::s_classTypeInfo(ParentClass::StaticClass());	\

#define MAD_IMPLEMENT_BASE_CLASS(BaseClass)										\
	const TTypeInfo BaseClass::s_classTypeInfo(nullptr);						\

// Actor specific macro definitions on top of base MAD class macro definitions
#define MAD_DECLARE_ACTOR(ClassName)	\
	MAD_DECLARE_CLASS(ClassName)		\

#define MAD_IMPLEMENT_ACTOR(ChildActorClass, ParentActorClass)	\
	MAD_IMPLEMENT_CLASS(ChildActorClass, ParentActorClass)		\

// Actor component specific macro definitions
#define MAD_DECLARE_COMPONENT(ComponentClass)					\
	MAD_DECLARE_CLASS(ComponentClass)							\
	
#define MAD_IMPLEMENT_COMPONENT(ChildComponentClass, ParentComponentClass)	\
	MAD_IMPLEMENT_CLASS(ChildComponentClass, ParentComponentClass)			\

#pragma endregion

#pragma region RTTI Queries
	// Reference Specialization
	template <typename IsAToClass, typename IsAFromClass>
	bool IsA(const IsAFromClass& inObject)
	{
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

	// Raw Ptr Specialization (has same implementation as reference specialization, clean up!)
	template <typename IsAToClass, typename IsAFromClass>
	bool IsA(const IsAFromClass* inObjectPtr)
	{
		// Similar to reference version
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
