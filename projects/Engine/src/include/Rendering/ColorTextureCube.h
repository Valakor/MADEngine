#pragma once

#include "Rendering/DepthTextureCube.h"

namespace MAD
{
	// Only supports loading vertical cross cube maps and rendering dynamic cube maps
	class UColorTextureCube
	{
	public:
		// Two methods of creating color texture cube:
		// 1) load from pre-created cube map file
		// 2) create backing texture to render to (TODO)
		UColorTextureCube();
		explicit UColorTextureCube(uint16_t inTexSideRes);
		explicit UColorTextureCube(const eastl::string& inTexturePath);

		void BindAsShaderResource(ETextureSlot inTextureSlot) const;
		void BindCubeSideAsTarget(uint8_t inCubeSide) const;
		ShaderResourcePtr_t GetShaderResource() const { return m_cubeSRV;}

		void SetClearColor(const Color& inClearColor) { m_clearColor = inClearColor; }
		Color GetClearColor() const { return m_clearColor; }
	private:
		bool m_bIsDynamic;
		Color m_clearColor;
		eastl::array<eastl::pair<DepthStencilPtr_t, RenderTargetPtr_t>, AsIntegral(ETextureCubeFace::MAX)> m_cubeOutputViews;
		ShaderResourcePtr_t m_cubeSRV;
		SGraphicsViewport m_viewPort;
	};
}
