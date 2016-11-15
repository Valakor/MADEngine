#include "Rendering/Texture.h"

#include "Core/GameEngine.h"
#include "Misc/AssetCache.h"
#include "Misc/Logging.h"
#include "Rendering/GraphicsDriver.h"
#include "Rendering/Renderer.h"

namespace MAD
{
	eastl::shared_ptr<UTexture> UTexture::Load(const eastl::string& inRelativePath, bool inLoadAsSRGB, bool inGenerateMips)
	{
		if (auto cachedTexture = UAssetCache::GetCachedResource<UTexture>(inRelativePath))
		{
			return cachedTexture;
		}

		eastl::string fullPath = UAssetCache::GetAssetRoot() + inRelativePath;

		uint64_t width, height;
		auto tex = gEngine->GetRenderer().GetGraphicsDriver().CreateTextureFromFile(fullPath, width, height, inLoadAsSRGB, inGenerateMips);
		if (!tex)
		{
			LOG(LogDefault, Warning, "Failed to load texture: `%s`. Falling back to default checker.png", inRelativePath.c_str());
			return GetDefaultTexture();
		}

		auto ret = eastl::make_shared<UTexture>();
		ret->m_width = width;
		ret->m_height = height;
		ret->m_textureSRV = tex;

		LOG(LogDefault, Log, "Loaded texture `%s`. sRGB=%i generateMips=%i\n", inRelativePath.c_str(), inLoadAsSRGB, inGenerateMips);
		UAssetCache::InsertResource<UTexture>(inRelativePath, ret);
		return ret;
	}

	eastl::shared_ptr<UTexture> UTexture::GetDefaultTexture()
	{
		static const eastl::string defaultTexture = "engine\\meshes\\primitives\\checker.png";

		if (auto cachedTexture = UAssetCache::GetCachedResource<UTexture>(defaultTexture))
		{
			return cachedTexture;
		}

		uint64_t width, height;
		auto tex = gEngine->GetRenderer().GetGraphicsDriver().CreateTextureFromFile(defaultTexture, width, height, true, true);
		if (!tex)
		{
			LOG(LogDefault, Error, "Failed to load default texture: `%s`", defaultTexture.c_str());
			return nullptr;
		}

		auto ret = eastl::make_shared<UTexture>();
		ret->m_width = width;
		ret->m_height = height;
		ret->m_textureSRV = tex;

		LOG(LogDefault, Log, "Loaded default texture `%s`. sRGB=1 generateMips=1\n", defaultTexture.c_str());
		UAssetCache::InsertResource<UTexture>(defaultTexture, ret);
		return ret;
	}

	UTexture::UTexture(): m_width(0)
	                    , m_height(0)
	                    , m_textureSRV() { }
}
