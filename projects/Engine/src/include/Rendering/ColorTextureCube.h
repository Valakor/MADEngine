#pragma once

#include "Rendering/GraphicsDriverTypes.h"
#include "Rendering/RenderingCommon.h"

#include <EASTL/array.h>

namespace MAD
{
	// Only supports loading vertical cross cube maps
	class UColorTextureCube
	{
	public:
		// Two methods of creating color texture cube:
		// 1) load from pre-created cube map file
		// 2) create backing texture to render to (TODO)
		UColorTextureCube() {}
		explicit UColorTextureCube(uint16_t inSideTexRes);
		explicit UColorTextureCube(const eastl::string& inTexturePath);

		void BindCubeSideAsTarget(uint8_t inCubeSide);

		void BindAsResource(ETextureSlot inTextureSlot);
	private:
		ShaderResourcePtr_t m_textureCubeSRV;

		D3D11_VIEWPORT m_textureViewport;
	};
}
