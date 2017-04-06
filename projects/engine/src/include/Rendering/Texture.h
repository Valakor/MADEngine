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
		static eastl::shared_ptr<UTexture> Load(const eastl::string& inRelativePath, bool inLoadAsSRGB, bool inGenerateMips, int32_t inMiscFlags = 0);

		// Load textures using UTexture::Load(...)
		UTexture();
		
		ShaderResourcePtr_t GetTexureResource() const { return m_textureSRV; }

		uint64_t GetWidth() const { return m_width; }
		uint64_t GetHeight() const { return m_height; }
	private:
		uint64_t m_width;
		uint64_t m_height;

		ShaderResourcePtr_t m_textureSRV;

		static eastl::shared_ptr<UTexture> GetDefaultTexture();
	};
}
