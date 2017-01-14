#include "Rendering/DepthTextureCube.h"
#include "Rendering/RenderContext.h"
#include "Rendering/GraphicsDriver.h"

namespace MAD
{
	UDepthTextureCube::UDepthTextureCube(uint16_t inTextureRes)
	{		
		// Create new viewport for the depth texture cube
		m_textureCubeViewport.TopLeftX = 0;
		m_textureCubeViewport.TopLeftY = 0;
		m_textureCubeViewport.Width = inTextureRes;
		m_textureCubeViewport.Height = inTextureRes;
		m_textureCubeViewport.MinDepth = 0.0f;
		m_textureCubeViewport.MaxDepth = 1.0f;

		ComPtr<ID3D11Device2> d3dDevice = URenderContext::Get().GetGraphicsDriver().TEMPGetDevice();

		// Create the backing texture for the texture cube
		ComPtr<ID3D11Texture2D> depthCubeTexture2D = nullptr;
		D3D11_TEXTURE2D_DESC depthCubeDesc;

		memset(&depthCubeDesc, 0, sizeof(depthCubeDesc));

		depthCubeDesc.Width = inTextureRes;
		depthCubeDesc.Height = inTextureRes;
		depthCubeDesc.MipLevels = 1;
		depthCubeDesc.ArraySize = UDepthTextureCube::s_numTextureCubeSides;
		depthCubeDesc.SampleDesc.Count = 1;
		depthCubeDesc.SampleDesc.Quality = 0;
		depthCubeDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
		depthCubeDesc.Usage = D3D11_USAGE_DEFAULT;
		depthCubeDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		depthCubeDesc.CPUAccessFlags = 0;
		depthCubeDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

		DX_HRESULT(d3dDevice->CreateTexture2D(&depthCubeDesc, nullptr, depthCubeTexture2D.GetAddressOf()), "Error: Creating backing texture for texture cube failed!");

		// Create the depth stencil view for each of the faces
		D3D11_DEPTH_STENCIL_VIEW_DESC depthCubeDSVDesc;

		memset(&depthCubeDSVDesc, 0, sizeof(depthCubeDSVDesc));

		depthCubeDSVDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthCubeDSVDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY; // Specifies how the render target will be accessed
		depthCubeDSVDesc.Texture2DArray.MipSlice = 0;
		depthCubeDSVDesc.Texture2DArray.ArraySize = 1; // We are only create render targets one at a time for each face

		for (uint8_t i = 0; i < UDepthTextureCube::s_numTextureCubeSides; ++i)
		{
			// Create a render target view for the current face
			depthCubeDSVDesc.Texture2DArray.FirstArraySlice = i;

			DX_HRESULT(d3dDevice->CreateDepthStencilView(depthCubeTexture2D.Get(), &depthCubeDSVDesc, m_textureCubeDSVs[i].GetAddressOf()), "Error: Creating depth stencil view for texture cube failed!");
		}

		// Create the shader resource view for the depth texture cube
		D3D11_SHADER_RESOURCE_VIEW_DESC textureCubeSRDesc;

		memset(&textureCubeSRDesc, 0, sizeof(textureCubeSRDesc));

		textureCubeSRDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		textureCubeSRDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		textureCubeSRDesc.TextureCube.MostDetailedMip = 0;
		textureCubeSRDesc.TextureCube.MipLevels = static_cast<UINT>(-1);

		DX_HRESULT(d3dDevice->CreateShaderResourceView(depthCubeTexture2D.Get(), &textureCubeSRDesc, m_textureCubeSRV.GetAddressOf()), "Error: Creating shader resource view for depth texture cube failed!");
	}

	void UDepthTextureCube::BindCubeSideAsTarget(uint8_t inCubeSide)
	{
		if (inCubeSide >= m_textureCubeDSVs.size())
		{
			return;
		}

		ComPtr<ID3D11DeviceContext> d3dDeviceContext = URenderContext::Get().GetGraphicsDriver().TEMPGetDeviceContext();

		// If we're binding a cube side as a depth stencil view, we must unbind it as a shader resource
		d3dDeviceContext->PSSetShaderResources(static_cast<UINT>(ETextureSlot::ShadowCube), 0, nullptr);

		// Clear the target depth stencil view
		d3dDeviceContext->ClearDepthStencilView(m_textureCubeDSVs[inCubeSide].Get(), 0, 1.0f, 0);

		// Bind the depth stencil view that corresponds with the target cube side
		d3dDeviceContext->OMSetRenderTargets(0, nullptr, m_textureCubeDSVs[inCubeSide].Get());
	
		// Change the viewport to match texture cube resolution
		d3dDeviceContext->RSSetViewports(1, &m_textureCubeViewport);
	}

	void UDepthTextureCube::BindAsResource(ETextureSlot inTextureSlot)
	{
		ComPtr<ID3D11DeviceContext> d3dDeviceContext = URenderContext::Get().GetGraphicsDriver().TEMPGetDeviceContext();

		// If we're binding the texture cube as a shader resource, we must unbind the depth stencil view
		d3dDeviceContext->OMSetRenderTargets(0, nullptr, nullptr);

		d3dDeviceContext->PSSetShaderResources(static_cast<UINT>(inTextureSlot), 1, m_textureCubeSRV.GetAddressOf());
	}
}