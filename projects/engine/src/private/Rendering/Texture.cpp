#include "Rendering/Texture.h"

#include "Core/GameEngine.h"
#include "Rendering/Renderer.h"

using Microsoft::WRL::ComPtr;

namespace MAD
{
	eastl::shared_ptr<UTexture> UTexture::Load(const eastl::string& inPath)
	{
		uint64_t width, height;
		auto tex = gEngine->GetRenderer().CreateTextureFromFile(inPath, width, height);
		if (!tex)
		{
			return nullptr;
		}

		auto ret = eastl::make_shared<UTexture>();
		ret->m_width = width;
		ret->m_height = height;
		ret->m_textureSRV = tex;

		return ret;
	}

	UTexture::UTexture(): m_width(0)
	                    , m_height(0)
	                    , m_textureSRV(nullptr) { }
}
