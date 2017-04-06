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
		explicit UColorTextureCube(const eastl::string& inTexturePath);

		void BindToPipeline(ETextureSlot inTextureSlot);
	private:
		Texture2DPtr_t m_texture2D;
		ShaderResourcePtr_t m_textureCubeSRV;
		uint32_t m_textureWidth;
		uint32_t m_textureHeight;
	};
}
