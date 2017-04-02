#include "Rendering/ColorTextureCube.h"
#include "Rendering/RenderContext.h"
#include "Rendering/GraphicsDriver.h"
#include "Rendering/Texture.h"

namespace MAD
{
	UColorTextureCube::UColorTextureCube(const eastl::string& inTexturePath)
	{
		eastl::shared_ptr<UTexture> textureCubeResource = UTexture::Load(inTexturePath, false, false);

		m_textureCubeSRV = textureCubeResource->GetTexureResource();
		m_textureWidth = textureCubeResource->GetWidth(); // Texture cube must have same width and height (square texture)
		m_textureHeight = textureCubeResource->GetHeight();

		MAD_ASSERT_DESC(m_textureCubeSRV, "Texture cube map wasn't created correctly!");
	}

	void UColorTextureCube::BindToPipeline(ETextureSlot inTextureSlot)
	{
		auto renderContext = URenderContext::Get().GetGraphicsDriver().TEMPGetDeviceContext();
	
		renderContext->PSSetShaderResources(AsIntegral(inTextureSlot), 1, m_textureCubeSRV.GetAddressOf());
	}
}
