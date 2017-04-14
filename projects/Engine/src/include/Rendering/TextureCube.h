#pragma once

#include "Rendering/GraphicsDriverTypes.h"
#include "Rendering/RenderingCommon.h"

#include <EASTl/array.h>

namespace MAD
{
	class UTextureCube
	{
	public:
		static const uint8_t Sides = 6;
	public:
		virtual void BindCubeSideAsTarget(uint8_t inCubeSide) = 0;
		
		void BindAsResource(ETextureSlot inTextureSlot);
		ResourcePtr_t GetSideResource(uint8_t inCubeSide) { return m_textureResourceViews[inCubeSide]; }
	protected:
		eastl::array<ResourcePtr_t, UTextureCube::Sides> m_textureResourceViews;
		ShaderResourcePtr_t m_textureCubeSRV;
		SGraphicsViewport m_textureViewport;
	};

	using TextureCubeVPArray_t = eastl::array<Matrix, UTextureCube::Sides>;
}
