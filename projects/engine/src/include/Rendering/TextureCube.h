#pragma once

#include "stdafx.h"
#include "Rendering/GraphicsDriverTypes.h"
#include <cstdint>

namespace MAD
{
	/*
		DirectX 11 TextureCube
		Requires:
		1. 6 Render Targets for each face of the cube
		2. 6 View Matrices for each face of the cube
		3. 1 DirectX 11 Texture that will be used as the backing texture of the texture cube
		4. 1 ShaderResourceView for the backing texture cube
		5. 1 Viewport based on the resolution of the texture cube resolution
	*/

	class UTextureCube
	{
	public:
		explicit UTextureCube(uint16_t inTextureRes);

		// Binds a cube side as the render target
		void BindCubeSideAsTarget(int inCubeSide);

		// Binds the entire texture cube as a shader resource at specified texture slot
		void BindAsResource(int inTextureSlot);
	private:

	private:
		static const uint8_t s_numTextureCubeSides = 6;

		eastl::array<SRenderTargetId, UTextureCube::s_numTextureCubeSides> m_textureCubeRTs;
		eastl::array<SDepthStencilId, UTextureCube::s_numTextureCubeSides> m_textureCubeDBs;
		eastl::array<Matrix, UTextureCube::s_numTextureCubeSides> m_textureCubeVMs;

		STexture2DId m_cubeTexture;
		SShaderResourceId m_cubeTextureSRV;
		uint16_t m_cubeTextureResolution;
	};
}