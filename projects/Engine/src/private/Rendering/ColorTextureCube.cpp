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

		// Create new viewport for the color texture cube
		m_viewPort.TopLeftX = 0;
		m_viewPort.TopLeftY = 0;
		m_viewPort.Width = inTexSideRes;
		m_viewPort.Height = inTexSideRes;
		m_viewPort.MinDepth = 0.0f;
		m_viewPort.MaxDepth = 1.0f;

		// Create the backing texture for the texture cube
		Texture2DPtr_t cubeRTTexture2D;
		Texture2DPtr_t cubeDSTexture2D;

		STexture2DDesc cubeRTDesc;
		STexture2DDesc cubeDSDesc;

		memset(&cubeRTDesc, 0, sizeof(cubeRTDesc));
		memset(&cubeDSDesc, 0, sizeof(cubeDSDesc));

		// Color Cube Render Target Texture-------------
		cubeRTDesc.Width = inTexSideRes;
		cubeRTDesc.Height = inTexSideRes;
		cubeRTDesc.MipLevels = 1;
		cubeRTDesc.ArraySize = AsIntegral(ETextureCubeFace::MAX);
		cubeRTDesc.SampleDesc.Count = 1;
		cubeRTDesc.SampleDesc.Quality = 0;
		cubeRTDesc.Format = DXGI_FORMAT_R32G32B32A32_TYPELESS;
		cubeRTDesc.Usage = AsRawType(EResourceUsage::Default);
		cubeRTDesc.BindFlags = AsIntegral(EBindFlag::RenderTarget | EBindFlag::ShaderResource);
		cubeRTDesc.CPUAccessFlags = 0;
		cubeRTDesc.MiscFlags = AsIntegral(EResourceMiscFlag::TextureCube);

		cubeRTTexture2D = graphicsDriver.CreateTexture2D(cubeRTDesc, nullptr);

		// Color Cube Depth Stencil Texture-------------
		cubeDSDesc.Width = inTexSideRes;
		cubeDSDesc.Height = inTexSideRes;
		cubeDSDesc.MipLevels = 1;
		cubeDSDesc.ArraySize = AsIntegral(ETextureCubeFace::MAX);
		cubeDSDesc.SampleDesc.Count = 1;
		cubeDSDesc.SampleDesc.Quality = 0;
		cubeDSDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
		cubeDSDesc.Usage = AsRawType(EResourceUsage::Default);
		cubeDSDesc.BindFlags = AsIntegral(EBindFlag::DepthStencil);
		cubeDSDesc.CPUAccessFlags = 0;
		cubeDSDesc.MiscFlags = AsIntegral(EResourceMiscFlag::TextureCube);

		cubeDSTexture2D = graphicsDriver.CreateTexture2D(cubeDSDesc, nullptr);
#ifdef _DEBUG
		cubeRTTexture2D.Debug_SetName("Color Cube Render Target");
		cubeDSTexture2D.Debug_SetName("Color Cube Depth Stencil");
#endif
		// Create the depth stencil and render target views for each of the faces
		SRenderTargetViewDesc cubeRTVDesc;
		SDepthStencilViewDesc cubeDSVDesc;

		memset(&cubeRTVDesc, 0, sizeof(cubeRTVDesc));
		memset(&cubeDSVDesc, 0, sizeof(cubeDSVDesc));

		cubeRTVDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		cubeRTVDesc.ViewDimension = AsRawType(ERTVDimension::Texture2DArray); // Specifies how the render target will be accessed
		cubeRTVDesc.Texture2DArray.MipSlice = 0;
		cubeRTVDesc.Texture2DArray.ArraySize = 1; // We only create render target views one at a time for each face

		cubeDSVDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		cubeDSVDesc.ViewDimension = AsRawType(EDSVDimension::Texture2DArray);
		cubeDSVDesc.Texture2DArray.MipSlice = 0;
		cubeDSVDesc.Texture2DArray.ArraySize = 1;

		for (uint8_t i = 0; i < AsIntegral(ETextureCubeFace::MAX); ++i)
		{
			// Create a render target view for the current face
			cubeRTVDesc.Texture2DArray.FirstArraySlice = i;
			cubeDSVDesc.Texture2DArray.FirstArraySlice = i;

			m_cubeOutputViews[i].first = graphicsDriver.CreateDepthStencil(cubeDSTexture2D, cubeDSVDesc);
			m_cubeOutputViews[i].second = graphicsDriver.CreateRenderTarget(cubeRTTexture2D, cubeRTVDesc);
		}

		// Create the shader resource view for the cube's render target only (we don't need to sample from depth cube because that is only used for z-buffering)
		m_cubeSRV = graphicsDriver.CreateShaderResource(cubeRTTexture2D, DXGI_FORMAT_R32G32B32A32_FLOAT, D3D11_SRV_DIMENSION_TEXTURECUBE, 0, static_cast<uint32_t>(-1));

		m_bIsDynamic = true;
	}

	// Create a color texture cube that is loaded with pre-baked cube map texture
	UColorTextureCube::UColorTextureCube(const eastl::string& inTexturePath)
	{
		eastl::shared_ptr<UTexture> cubeTexture = UTexture::Load(inTexturePath, true, false, D3D11_RESOURCE_MISC_TEXTURECUBE);

		m_cubeSRV = cubeTexture->GetTexureResource();

		MAD_ASSERT_DESC(m_cubeSRV, "Error with loading or creating the DX texture cube");

		m_bIsDynamic = false;
	}

	void UColorTextureCube::BindAsShaderResource(ETextureSlot inTextureSlot) const
	{
		auto& graphicsDriver = URenderContext::Get().GetGraphicsDriver();

		// If we're binding the SRV for the cube map, we must unbind the render target view (not also the depth stencil view, but might as well)
		graphicsDriver.SetRenderTargets(nullptr, 0, nullptr);
		graphicsDriver.SetPixelShaderResource(m_cubeSRV, inTextureSlot);
	}

	void UColorTextureCube::BindCubeSideAsTarget(uint8_t inCubeSide) const
	{
		MAD_ASSERT_DESC(inCubeSide < m_cubeOutputViews.size(), "Error: Invalid side number");
		MAD_ASSERT_DESC(m_bIsDynamic, "Error: You may not bind a pre-loaded cube map as a render target");

		if (inCubeSide >= m_cubeOutputViews.size() || !m_bIsDynamic)
		{
			return;
		}

		auto& graphicsDriver = URenderContext::Get().GetGraphicsDriver();

		graphicsDriver.SetPixelShaderResource(nullptr, ETextureSlot::CubeMap); // If we're binding a cube side as a render target view, we must unbind it as a shader resource
		graphicsDriver.ClearDepthStencil(m_cubeOutputViews[inCubeSide].first, true, 1.0f);
		graphicsDriver.ClearRenderTarget(m_cubeOutputViews[inCubeSide].second, m_clearColor);
		graphicsDriver.SetRenderTargets(&m_cubeOutputViews[inCubeSide].second, 1, m_cubeOutputViews[inCubeSide].first); // Bind the render target and depth stencil view that corresponds with the target cube side
		graphicsDriver.SetViewport(m_viewPort); // Change the viewport to match texture cube resolution
	}
}
