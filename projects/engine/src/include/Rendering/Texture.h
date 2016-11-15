#pragma once

#include <EASTL/string.h>
#include <EASTL/shared_ptr.h>

#include "Rendering/GraphicsDriverTypes.h"

namespace MAD
{
	class UTexture
	{
	public:
		/*
		 * Loads a texture at the given path. The path should be relative to the assets root directory. Internally uses a cache
		 * to ensure textures are only loaded once. Will load a default (checker pattern) texture if the texture at the given
		 * path could not be loaded.
		 */
		static eastl::shared_ptr<UTexture> Load(const eastl::string& inRelativePath, bool inLoadAsSRGB, bool inGenerateMips);

		// Load textures using UTexture::Load(...)
		UTexture();
		
		SShaderResourceId GetTexureResourceId() const { return m_textureSRV; }

	private:
		uint64_t m_width;
		uint64_t m_height;

		SShaderResourceId m_textureSRV;

		static eastl::shared_ptr<UTexture> GetDefaultTexture();
	};
}
