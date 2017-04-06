#include "Rendering/ColorTextureCube.h"
#include "Rendering/RenderContext.h"
#include "Rendering/GraphicsDriver.h"
#include "Rendering/Texture.h"
#include "Misc/AssetCache.h"
#include "Misc/utf8conv.h"

#include <DirectXTK/DDSTextureLoader.h>

#include <fstream>
#include <algorithm>
#include <iterator>

namespace MAD
{
	UColorTextureCube::UColorTextureCube(const eastl::string& inTexturePath)
	{
		eastl::shared_ptr<UTexture> cubeTexture = UTexture::Load(inTexturePath, true, false, D3D11_RESOURCE_MISC_TEXTURECUBE);

		m_textureCubeSRV = cubeTexture->GetTexureResource();

		MAD_ASSERT_DESC(m_textureCubeSRV, "Error with loading or creating the DX texture cube");
	}

	void UColorTextureCube::BindToPipeline(ETextureSlot inTextureSlot)
	{
		auto renderContext = URenderContext::Get().GetGraphicsDriver().TEMPGetDeviceContext();
	
		renderContext->PSSetShaderResources(AsIntegral(inTextureSlot), 1, m_textureCubeSRV.GetAddressOf());
	}
}
