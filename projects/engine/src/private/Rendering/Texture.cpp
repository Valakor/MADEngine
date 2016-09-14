#include "Rendering/Texture.h"

#include "Core/GameEngine.h"
#include "Misc/Logging.h"
#include "Rendering/Renderer.h"

namespace MAD
{
	eastl::shared_ptr<UTexture> UTexture::Load(const eastl::string& inPath)
	{
		uint64_t width, height;
		auto tex = gEngine->GetRenderer().CreateTextureFromFile(inPath, width, height);
		if (!tex)
		{
			LOG(LogDefault, Warning, "Failed to load texture: `%s`. Falling back to default checker.png", inPath.c_str());
			tex = gEngine->GetRenderer().CreateTextureFromFile(".\\assets\\engine\\meshes\\primitives\\checker.png", width, height);
		}

		auto ret = eastl::make_shared<UTexture>();
		ret->m_width = width;
		ret->m_height = height;
		ret->m_textureSRV = tex;

		return ret;
	}

	UTexture::UTexture(): m_width(0)
	                    , m_height(0)
	                    , m_textureSRV() { }
}
