#pragma once

#include "Rendering/GraphicsDriverTypes.h"
#include "Rendering/RenderingCommon.h"

namespace MAD
{
	class UColorTextureCube
	{
	public:
		// Two methods of creating color texture cube:
		// 1) load from pre-created cube map file
		// 2) create backing texture to render to (TODO)
		explicit UColorTextureCube(const eastl::string& inTexturePath);

		void BindToPipeline(ETextureSlot inTextureSlot);
	private:
		ShaderResourcePtr_t m_textureCubeSRV;
		uint64_t m_textureWidth;
		uint64_t m_textureHeight;
	};
}
