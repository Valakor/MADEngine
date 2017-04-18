#pragma once

#include "Rendering/TextureCube.h"

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
		UDepthTextureCube() {}
		explicit UDepthTextureCube(uint16_t inTextureRes);

		void BindAsShaderResource(ETextureSlot inTextureSlot) const;

		// Binds a cube side as the depth stencil view
		void BindCubeSideAsTarget(uint8_t inCubeSide) const;
	private:
		eastl::array<DepthStencilPtr_t, AsIntegral(ETextureCubeFace::MAX)> m_depthCubeDSVs;
		ShaderResourcePtr_t m_depthCubeSRV;
		SGraphicsViewport m_viewPort;
	};
}
