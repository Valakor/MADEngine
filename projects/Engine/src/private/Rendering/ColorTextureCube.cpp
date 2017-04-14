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
	UColorTextureCube::UColorTextureCube() : m_bIsDynamic(false) {}

	// Create a color texture cube that can be rendered to where each side is inSideTexRes width and height
	UColorTextureCube::UColorTextureCube(uint16_t inTexSideRes)
	{
		auto& graphicsDriver = URenderContext::Get().GetGraphicsDriver();

		// Create new viewport for the depth texture cube
		m_textureViewport.TopLeftX = 0;
		m_textureViewport.TopLeftY = 0;
		m_textureViewport.Width = inTexSideRes;
		m_textureViewport.Height = inTexSideRes;
		m_textureViewport.MinDepth = 0.0f;
		m_textureViewport.MaxDepth = 1.0f;

		// Create the backing texture for the texture cube
		Texture2DPtr_t colorCubeTexture2D;
		STexture2DDesc textureCubeDesc;

		memset(&textureCubeDesc, 0, sizeof(textureCubeDesc));

		textureCubeDesc.Width = inTexSideRes;
		textureCubeDesc.Height = inTexSideRes;
		textureCubeDesc.MipLevels = 1;
		textureCubeDesc.ArraySize = UTextureCube::Sides;
		textureCubeDesc.SampleDesc.Count = 1;
		textureCubeDesc.SampleDesc.Quality = 0;
		textureCubeDesc.Format = DXGI_FORMAT_R32G32B32A32_TYPELESS;
		textureCubeDesc.Usage = AsRawType(EResourceUsage::Default);
		textureCubeDesc.BindFlags = static_cast<UINT>(EBindFlag::RenderTarget | EBindFlag::ShaderResource);
		textureCubeDesc.CPUAccessFlags = 0;
		textureCubeDesc.MiscFlags = static_cast<UINT>(EResourceMiscFlag::TextureCube);

		colorCubeTexture2D = graphicsDriver.CreateTexture2D(textureCubeDesc, nullptr);
#ifdef _DEBUG
		colorCubeTexture2D.Debug_SetName("Color Cube");
#endif
		// Create the depth stencil view for each of the faces
		SRenderTargetViewDesc textureCubeRTDesc;

		memset(&textureCubeRTDesc, 0, sizeof(textureCubeRTDesc));

		textureCubeRTDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		textureCubeRTDesc.ViewDimension = AsRawType(ERTVDimension::Texture2DArray); // Specifies how the render target will be accessed
		textureCubeRTDesc.Texture2DArray.MipSlice = 0;
		textureCubeRTDesc.Texture2DArray.ArraySize = 1; // We only create render target views one at a time for each face

		for (uint8_t i = 0; i < UTextureCube::Sides; ++i)
		{
			// Create a render target view for the current face
			textureCubeRTDesc.Texture2DArray.FirstArraySlice = i;

			m_textureResourceViews[i] = graphicsDriver.CreateRenderTarget(colorCubeTexture2D, textureCubeRTDesc);
		}

		// Create the shader resource view for the depth texture cube
		m_textureCubeSRV = graphicsDriver.CreateShaderResource(colorCubeTexture2D, DXGI_FORMAT_R32G32B32A32_FLOAT, D3D11_SRV_DIMENSION_TEXTURECUBE, 0, static_cast<uint32_t>(-1));

		m_bIsDynamic = true;
	}

	// Create a color texture cube that is loaded with pre-baked cube map texture
	UColorTextureCube::UColorTextureCube(const eastl::string& inTexturePath)
	{
		eastl::shared_ptr<UTexture> cubeTexture = UTexture::Load(inTexturePath, true, false, D3D11_RESOURCE_MISC_TEXTURECUBE);

		m_textureCubeSRV = cubeTexture->GetTexureResource();

		MAD_ASSERT_DESC(m_textureCubeSRV, "Error with loading or creating the DX texture cube");

		m_bIsDynamic = false;
	}

	void UColorTextureCube::BindCubeSideAsTarget(uint8_t inCubeSide)
	{
		MAD_ASSERT_DESC(inCubeSide < m_textureResourceViews.size(), "Error: Invalid side number");
		MAD_ASSERT_DESC(m_bIsDynamic, "Error: You may not bind a pre-loaded cube map as a render target");

		if (inCubeSide >= m_textureResourceViews.size() || !m_bIsDynamic)
		{
			return;
		}

		auto& graphicsDriver = URenderContext::Get().GetGraphicsDriver();
		RenderTargetPtr_t cubeSideRenderTarget = m_textureResourceViews[inCubeSide];

		graphicsDriver.SetPixelShaderResource(nullptr, ETextureSlot::CubeMap); // If we're binding a cube side as a render target view, we must unbind it as a shader resource
		graphicsDriver.ClearRenderTarget(m_textureResourceViews[inCubeSide], m_clearColor);
		graphicsDriver.SetRenderTargets(&cubeSideRenderTarget, 1, nullptr); // Bind the render target view view that corresponds with the target cube side
		graphicsDriver.SetViewport(m_textureViewport.TopLeftX, m_textureViewport.TopLeftY, m_textureViewport.Width, m_textureViewport.Height); // Change the viewport to match texture cube resolution
	}
}
