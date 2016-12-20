#pragma once

#include <EASTL/functional.h>
#include <EASTL/numeric_limits.h>

#include "Misc/Assert.h"

namespace MAD
{
	template <class DerivedType, typename IdType>
	struct SGraphicsObjectId
	{
	public:
		static const DerivedType Invalid;

		inline bool IsValid() const { return m_Id != INVALID_ID; }

		inline explicit operator bool() const
		{
			return IsValid();
		}

		inline void Invalidate() { m_Id = INVALID_ID; }

		inline friend bool operator==(SGraphicsObjectId<DerivedType, IdType> lhs, SGraphicsObjectId<DerivedType, IdType> rhs)
		{
			return lhs.m_Id == rhs.m_Id;
		}

		inline friend bool operator!=(SGraphicsObjectId<DerivedType, IdType> lhs, SGraphicsObjectId<DerivedType, IdType> rhs)
		{
			return !(lhs == rhs);
		}

	protected:
		friend class UGraphicsDriver;
		static_assert(eastl::is_unsigned<IdType>::value, "IdType must be an unsigned integral type");

		SGraphicsObjectId() : m_Id(INVALID_ID) { }
		explicit SGraphicsObjectId(IdType inId) : m_Id(inId) { }

		inline static DerivedType Next()
		{
			DerivedType id(s_counter++);
			MAD_ASSERT_DESC(id.IsValid(), "Newly-created GraphicsObjectId must have a valid ID. Likely invalid because you've hit max ID #.");
			return id;
		}

		static const IdType INVALID_ID = eastl::numeric_limits<IdType>::max();
		static IdType s_counter;
		IdType m_Id;
	};

	template <class DerivedType, typename IdType>
	IdType SGraphicsObjectId<DerivedType, IdType>::s_counter = 0;

	template <class DerivedType, typename IdType>
	const DerivedType SGraphicsObjectId<DerivedType, IdType>::Invalid(INVALID_ID);
}

#define DECLARE_GRAPHICS_OBJ_ID(name, type)				\
namespace MAD											\
{														\
	struct name: public SGraphicsObjectId<name, type>	\
	{													\
	public:												\
		name(): Super_t() { }								\
	private:											\
		using Super_t = SGraphicsObjectId<name, type>;	\
		friend struct SGraphicsObjectId<name, type>;	\
		friend struct eastl::hash<name>;				\
		friend class UGraphicsDriver;					\
		name(type inId) : Super_t(inId) { }				\
	};													\
	static_assert(sizeof(name) == sizeof(type), "");	\
}														\
namespace eastl											\
{														\
	using namespace MAD;								\
	template <> struct hash<name>						\
	{													\
		size_t operator()(name val) const				\
		{												\
			return static_cast<size_t>(val.m_Id);		\
		}												\
	};													\
}						

DECLARE_GRAPHICS_OBJ_ID(SVertexShaderId, uint8_t)
DECLARE_GRAPHICS_OBJ_ID(SGeometryShaderId, uint8_t)
DECLARE_GRAPHICS_OBJ_ID(SPixelShaderId, uint8_t)
DECLARE_GRAPHICS_OBJ_ID(SInputLayoutId, uint8_t)
DECLARE_GRAPHICS_OBJ_ID(SRenderTargetId, uint16_t)
DECLARE_GRAPHICS_OBJ_ID(SDepthStencilId, uint8_t)
DECLARE_GRAPHICS_OBJ_ID(SDepthStencilStateId, uint8_t)
DECLARE_GRAPHICS_OBJ_ID(SBlendStateId, uint8_t)
DECLARE_GRAPHICS_OBJ_ID(SSamplerStateId, uint8_t)
DECLARE_GRAPHICS_OBJ_ID(SRasterizerStateId, uint8_t)
DECLARE_GRAPHICS_OBJ_ID(STexture2DId, uint16_t)
DECLARE_GRAPHICS_OBJ_ID(SShaderResourceId, uint16_t)
DECLARE_GRAPHICS_OBJ_ID(SBufferId, uint16_t)

#undef DECLARE_GRAPHICS_OBJ_ID
