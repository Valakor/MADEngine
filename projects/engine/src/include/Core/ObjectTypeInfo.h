#pragma once

#include <stdint.h>

namespace MAD
{
	using TypeID = uint32_t;

	class TTypeInfo
	{
	public:
		static TypeID s_currentTypeID;
	public:
		explicit TTypeInfo(const TTypeInfo* inParent);

		inline TypeID GetTypeID() const { return m_typeID; }
		inline const TTypeInfo* GetParent() const { return m_parent; }

	private:
		const TypeID m_typeID;
		const TTypeInfo* m_parent;
	};

#pragma region Macro Definitions
// Base object class macro definition
#define MAD_DECLARE_BASE_CLASS(BaseClass)									\
	public:																	\
		static const TTypeInfo* StaticClass()								\
		{																	\
			static TTypeInfo s_classTypeInfo(nullptr);						\
			return &s_classTypeInfo;											\
		}																	\
	private:																\

// Actor specific macro definition
#define MAD_DECLARE_CLASS(ClassName, ParentClass)							\
	public:																	\
		static const TTypeInfo* StaticClass()								\
		{																	\
			static TTypeInfo s_classTypeInfo(ParentClass::StaticClass());	\
			return &s_classTypeInfo;											\
		}																	\
	private:																\
		using Super = ParentClass;											\

// Actor specific macro definitions on top of base MAD class macro definitions
#define MAD_DECLARE_ACTOR(ClassName, ParentClass)							\
	MAD_DECLARE_CLASS(ClassName, ParentClass)								\

// Actor component specific macro definitions
#define MAD_DECLARE_COMPONENT(ComponentClass, ParentClass)					\
	MAD_DECLARE_CLASS(ComponentClass, ParentClass)							\
	
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
