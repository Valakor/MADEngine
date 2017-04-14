#include "Rendering/DepthTextureCube.h"
#include "Rendering/RenderContext.h"
#include "Rendering/GraphicsDriver.h"

namespace MAD
{
	UDepthTextureCube::UDepthTextureCube(uint16_t inTextureRes)
	{
		auto& graphicsDriver = URenderContext::Get().GetGraphicsDriver();

		// Create new viewport for the depth texture cube
		m_textureViewport.TopLeftX = 0;
		m_textureViewport.TopLeftY = 0;
		m_textureViewport.Width = inTextureRes;
		m_textureViewport.Height = inTextureRes;
		m_textureViewport.MinDepth = 0.0f;
		m_textureViewport.MaxDepth = 1.0f;

		// Create the backing texture for the texture cube
		Texture2DPtr_t depthCubeTexture2D;
		STexture2DDesc depthCubeDesc;

		memset(&depthCubeDesc, 0, sizeof(depthCubeDesc));

		depthCubeDesc.Width = inTextureRes;
		depthCubeDesc.Height = inTextureRes;
		depthCubeDesc.MipLevels = 1;
		depthCubeDesc.ArraySize = UTextureCube::Sides;
		depthCubeDesc.SampleDesc.Count = 1;
		depthCubeDesc.SampleDesc.Quality = 0;
		depthCubeDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
		depthCubeDesc.Usage = AsRawType(EResourceUsage::Default);
		depthCubeDesc.BindFlags = static_cast<UINT>(EBindFlag::DepthStencil | EBindFlag::ShaderResource);
		depthCubeDesc.CPUAccessFlags = 0;
		depthCubeDesc.MiscFlags = static_cast<UINT>(EResourceMiscFlag::TextureCube);

		depthCubeTexture2D = graphicsDriver.CreateTexture2D(depthCubeDesc, nullptr);
#ifdef _DEBUG
		depthCubeTexture2D.Debug_SetName("Depth Cube");
#endif
		// Create the depth stencil view for each of the faces
		SDepthStencilViewDesc depthCubeDSVDesc;

		memset(&depthCubeDSVDesc, 0, sizeof(depthCubeDSVDesc));

		depthCubeDSVDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthCubeDSVDesc.ViewDimension = AsRawType(EDSVDimension::Texture2DArray); // Specifies how the render target will be accessed
		depthCubeDSVDesc.Texture2DArray.MipSlice = 0;
		depthCubeDSVDesc.Texture2DArray.ArraySize = 1; // We only create depth stencil views one at a time for each face

		for (uint8_t i = 0; i < UTextureCube::Sides; ++i)
		{
			// Create a render target view for the current face
			depthCubeDSVDesc.Texture2DArray.FirstArraySlice = i;

			m_textureResourceViews[i] = graphicsDriver.CreateDepthStencil(depthCubeTexture2D, depthCubeDSVDesc);
		}

		// Create the shader resource view for the depth texture cube
		m_textureCubeSRV = graphicsDriver.CreateShaderResource(depthCubeTexture2D, DXGI_FORMAT_R24_UNORM_X8_TYPELESS, D3D11_SRV_DIMENSION_TEXTURECUBE, 0, static_cast<uint32_t>(-1));
	}

	void UDepthTextureCube::BindCubeSideAsTarget(uint8_t inCubeSide)
	{
		if (inCubeSide >= m_textureResourceViews.size())
		{
			return;
		}

		auto& graphicsDriver = URenderContext::Get().GetGraphicsDriver();

		// If we're binding a cube side as a depth stencil view, we must unbind it as a shader resource
		graphicsDriver.SetPixelShaderResource(nullptr, ETextureSlot::CubeMap);

		// Clear the target depth stencil view
		graphicsDriver.ClearDepthStencil(m_textureResourceViews[inCubeSide], true, 1.0);

		// Bind the depth stencil view that corresponds with the target cube side
		graphicsDriver.SetRenderTargets(nullptr, 0, m_textureResourceViews[inCubeSide]);
	
		// Change the viewport to match texture cube resolution
		graphicsDriver.SetViewport(m_textureViewport.TopLeftX, m_textureViewport.TopLeftY, m_textureViewport.Width, m_textureViewport.Height);
	}
}