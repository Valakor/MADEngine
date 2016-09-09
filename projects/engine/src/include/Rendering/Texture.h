#pragma once

#include <EASTL/string.h>
#include <EASTL/shared_ptr.h>

#include "Rendering/GraphicsDriverTypes.h"

namespace MAD
{
	class UTexture
	{
	public:
		// Load textures using UAssetCache::Load<UTexture>(...)
		UTexture();

	private:
		friend class UAssetCache;
		static eastl::shared_ptr<UTexture> Load(const eastl::string& inPath);

		uint64_t m_width;
		uint64_t m_height;

		SShaderResourceId m_textureSRV;
	};
}
