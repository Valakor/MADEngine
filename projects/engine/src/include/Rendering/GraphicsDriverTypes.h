#pragma once

#include <EASTL/functional.h>
#include <EASTL/numeric_limits.h>

#include "Misc/Assert.h"

#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

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

//DECLARE_GRAPHICS_OBJ_ID(SVertexShaderId, uint8_t)
//DECLARE_GRAPHICS_OBJ_ID(SGeometryShaderId, uint8_t)
//DECLARE_GRAPHICS_OBJ_ID(PixelShaderPtr_t, uint8_t)
//DECLARE_GRAPHICS_OBJ_ID(InputLayoutPtr_t, uint8_t)
//DECLARE_GRAPHICS_OBJ_ID(RenderTargetPtr_t, uint16_t)
//DECLARE_GRAPHICS_OBJ_ID(DepthStencilPtr_t, uint8_t)
//DECLARE_GRAPHICS_OBJ_ID(DepthStencilStatePtr_t, uint8_t)
//DECLARE_GRAPHICS_OBJ_ID(BlendStatePtr_t, uint8_t)
//DECLARE_GRAPHICS_OBJ_ID(SamplerStatePtr_t, uint8_t)
//DECLARE_GRAPHICS_OBJ_ID(RasterizerStatePtr_t, uint8_t)
//DECLARE_GRAPHICS_OBJ_ID(Texture2DPtr_t, uint16_t)
DECLARE_GRAPHICS_OBJ_ID(SShaderResourceId, uint16_t)
DECLARE_GRAPHICS_OBJ_ID(SBufferId, uint16_t)

using VertexShaderPtr_t = ComPtr<ID3D11VertexShader>;
using GeometryShaderPtr_t = ComPtr<ID3D11GeometryShader>;
using PixelShaderPtr_t = ComPtr<ID3D11PixelShader>;
using InputLayoutPtr_t = ComPtr<ID3D11InputLayout>;
using RenderTargetPtr_t = ComPtr<ID3D11RenderTargetView>;
using DepthStencilPtr_t = ComPtr<ID3D11DepthStencilView>;
using DepthStencilStatePtr_t = ComPtr<ID3D11DepthStencilState>;
using BlendStatePtr_t = ComPtr<ID3D11BlendState1>;
using SamplerStatePtr_t = ComPtr<ID3D11SamplerState>;
using RasterizerStatePtr_t = ComPtr<ID3D11RasterizerState1>;
//using ShaderResourcePtr_t = ComPtr<ID3D11ShaderResourceView>;

#undef DECLARE_GRAPHICS_OBJ_ID
