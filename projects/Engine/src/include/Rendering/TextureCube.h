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
	protected:
		eastl::array<ResourcePtr_t, 6> m_textureResourceViews;
		ShaderResourcePtr_t m_textureCubeSRV;
		SGraphicsViewport m_textureViewport;
	};

	using TextureCubeVPArray_t = eastl::array<Matrix, UTextureCube::Sides>;
}
