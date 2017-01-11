#pragma once

#include "Rendering/GraphicsDriverTypes.h"
#include "Rendering/RenderingCommon.h"

#include <cstdint>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

namespace MAD
{
	/*
		DirectX 11 TextureCube - Experiential (not using normal UGraphicsDriver API b/c that API is very inflexible with setting up properties for different graphics primitives
		Requires:
		* 6 Render Targets for each face of the cube
		* 1 ShaderResourceView for the backing texture cube
		* 1 Viewport based on the resolution of the texture cube resolution
	*/

	class UDepthTextureCube
	{
	public:
		static const uint8_t s_numTextureCubeSides = 6;
	public:
		explicit UDepthTextureCube(uint16_t inTextureRes);

		// Binds a cube side as the render target
		void BindCubeSideAsTarget(uint8_t inCubeSide);

		// Binds the entire texture cube as a shader resource at specified texture slot
		void BindAsResource(ETextureSlot inTextureSlot);
	private:
		eastl::array<ComPtr<ID3D11DepthStencilView>, UDepthTextureCube::s_numTextureCubeSides> m_textureCubeDSVs;
		
		ComPtr<ID3D11ShaderResourceView> m_textureCubeSRV;

		D3D11_VIEWPORT m_textureCubeViewport;
	};

	using TextureCubeVPArray_t = eastl::array<Matrix, UDepthTextureCube::s_numTextureCubeSides>;
}