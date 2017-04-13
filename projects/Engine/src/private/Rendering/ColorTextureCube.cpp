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
	// Create a color texture cube that can be rendered to where each side is inSideTexRes width and height
	UColorTextureCube::UColorTextureCube(uint16_t inSideTexRes)
	{
		UNREFERENCED_PARAMETER(inSideTexRes);

	}

	UColorTextureCube::UColorTextureCube(const eastl::string& inTexturePath)
	{
		eastl::shared_ptr<UTexture> cubeTexture = UTexture::Load(inTexturePath, true, false, D3D11_RESOURCE_MISC_TEXTURECUBE);

		m_textureCubeSRV = cubeTexture->GetTexureResource();

		MAD_ASSERT_DESC(m_textureCubeSRV, "Error with loading or creating the DX texture cube");
	}

	void UColorTextureCube::BindCubeSideAsTarget(uint8_t inCubeSide)
	{
		UNREFERENCED_PARAMETER(inCubeSide);
	}

	void UColorTextureCube::BindAsResource(ETextureSlot inTextureSlot)
	{
		auto renderContext = URenderContext::Get().GetGraphicsDriver().TEMPGetDeviceContext();
	
		renderContext->PSSetShaderResources(AsIntegral(inTextureSlot), 1, m_textureCubeSRV.GetAddressOf());
	}
}
