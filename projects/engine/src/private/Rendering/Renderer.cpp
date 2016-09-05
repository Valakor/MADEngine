#include "Rendering/Renderer.h"

#include <DirectXTK/DDSTextureLoader.h>
#include <DirectXTK/WICTextureLoader.h>

#include "Core/GameWindow.h"
#include "Misc/Assert.h"
#include "Misc/Logging.h"
#include "Misc/utf8conv.h"

using Microsoft::WRL::ComPtr;

namespace MAD
{
	DECLARE_LOG_CATEGORY(LogRenderer);
	DECLARE_LOG_CATEGORY(LogTextureImport);

#ifdef _DEBUG
#define HR_ASSERT_SUCCESS(hr, desc) MAD_ASSERT_DESC(SUCCEEDED(hr), desc)
#else
#define HR_ASSERT_SUCCESS(hr, desc) (void)(hr)
#endif

	namespace
	{
		ComPtr<ID3D11Device2>			g_d3dDevice;
		ComPtr<ID3D11DeviceContext2>	g_d3dDeviceContext;
		ComPtr<IDXGISwapChain2>			g_dxgiSwapChain;
		ComPtr<ID3D11RenderTargetView>	g_d3dBackBufferRenderTarget;

		const UINT g_swapChainBufferCount = 3;
		const DXGI_FORMAT g_swapChainFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

		void CreateDevice()
		{
			UINT createDeviceFlags = D3D11_CREATE_DEVICE_SINGLETHREADED;
#ifdef _DEBUG
			createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

			// To make it easy on us, we're only going to support D3D 11.2 (Windows 8.1 and above)
			D3D_FEATURE_LEVEL featureLevels[] =
			{
				D3D_FEATURE_LEVEL_11_1,
			};
			UINT numFeatureLevels = ARRAYSIZE(featureLevels);

			D3D_FEATURE_LEVEL supportedFeatureLevel;
			ComPtr<ID3D11Device> d3dDevice0;
			ComPtr<ID3D11DeviceContext> d3dDeviceContext0;

			HRESULT hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
									D3D11_SDK_VERSION, d3dDevice0.ReleaseAndGetAddressOf(), &supportedFeatureLevel, d3dDeviceContext0.ReleaseAndGetAddressOf());
			HR_ASSERT_SUCCESS(hr, "Failed to create D3D device with feature level 11_1");

			g_d3dDevice.Reset();
			hr = d3dDevice0.As(&g_d3dDevice);
			HR_ASSERT_SUCCESS(hr, "Failed to get device as D3d 11_2");

			g_d3dDeviceContext.Reset();
			hr = d3dDeviceContext0.As(&g_d3dDeviceContext);
			HR_ASSERT_SUCCESS(hr, "Failed to get device context as D3d 11_2");
		}

		HRESULT CreateSwapChain(HWND inWindow)
		{
			(void)inWindow;

			HRESULT hr;

			ComPtr<IDXGIDevice3> dxgiDevice;
			hr = g_d3dDevice.As(&dxgiDevice);
			HR_ASSERT_SUCCESS(hr, "Failed to get DXGI device from D3D device");

			ComPtr<IDXGIAdapter2> dxgiAdapter;
			hr = dxgiDevice->GetParent(__uuidof(IDXGIAdapter2), &dxgiAdapter);
			HR_ASSERT_SUCCESS(hr, "Failed to get DXGI adapter from DXGI device");

			ComPtr<IDXGIFactory3> dxgiFactory;
			hr = dxgiAdapter->GetParent(_uuidof(IDXGIFactory3), &dxgiFactory);
			HR_ASSERT_SUCCESS(hr, "Failed to get DXGI factory from DXGI adapter");

			DXGI_SWAP_CHAIN_DESC1 scd = { 0 };
			scd.Width = 0;
			scd.Height = 0;
			scd.Format = g_swapChainFormat;
			scd.Stereo = FALSE;
			scd.SampleDesc.Count = 1;
			scd.SampleDesc.Quality = 0;
			scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			scd.BufferCount = g_swapChainBufferCount;
			scd.Scaling = DXGI_SCALING_NONE;
			scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
			scd.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
			scd.Flags = 0;

			DXGI_SWAP_CHAIN_FULLSCREEN_DESC scfd = { 0 };
			scfd.RefreshRate = { 0, 0 };
			scfd.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
			scfd.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
			scfd.Windowed = TRUE;

			ComPtr<IDXGISwapChain1> swapChain;
			hr = dxgiFactory->CreateSwapChainForHwnd(g_d3dDevice.Get(), inWindow, &scd, &scfd, nullptr, swapChain.ReleaseAndGetAddressOf());
			HR_ASSERT_SUCCESS(hr, "Failed to create swap chain");

			g_dxgiSwapChain.Reset();
			hr = swapChain.As(&g_dxgiSwapChain);
			HR_ASSERT_SUCCESS(hr, "Failed to get SwapChain1 as SwapChain2");

			return hr;
		}

		void CreateBackBufferRenderTargetView()
		{
			ComPtr<ID3D11Texture2D> backBuffer;
			HRESULT hr = g_dxgiSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf()));
			HR_ASSERT_SUCCESS(hr, "Failed to get back buffer from swap chain");

			hr = g_d3dDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, g_d3dBackBufferRenderTarget.ReleaseAndGetAddressOf());
			HR_ASSERT_SUCCESS(hr, "Failed to create render target view from back buffer");

			g_d3dDeviceContext->OMSetRenderTargets(1, g_d3dBackBufferRenderTarget.GetAddressOf(), nullptr);
		}
	}

	URenderer::URenderer(): m_window(nullptr) { }

	bool URenderer::Init(UGameWindow& inWindow)
	{
		LOG(LogRenderer, Log, "Renderer initialization begin...\n");

		m_window = &inWindow;

		CreateDevice();

		CreateSwapChain(m_window->GetHWnd());

		CreateBackBufferRenderTargetView();

		SetViewport(m_window->GetClientSize());

		LOG(LogRenderer, Log, "Renderer initialization successful\n");
		return true;
	}

	void URenderer::Shutdown()
	{
		if (g_d3dDeviceContext)
		{
			g_d3dDeviceContext->ClearState();
			g_d3dDeviceContext->Flush();
		}

		g_dxgiSwapChain->SetFullscreenState(FALSE, nullptr);

		g_d3dBackBufferRenderTarget.Reset();
		g_dxgiSwapChain.Reset();
		g_d3dDeviceContext.Reset();
		g_d3dDevice.Reset();
	}

	void URenderer::OnScreenSizeChanged()
	{
		if (!g_d3dDeviceContext) return;

		auto newSize = m_window->GetClientSize();
		LOG(LogRenderer, Log, "OnScreenSizeChanged: { %i, %i }\n", newSize.x, newSize.y);

		g_d3dDeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
		g_d3dBackBufferRenderTarget.Reset();
		g_d3dDeviceContext->Flush();
		g_d3dDeviceContext->ClearState();

		HRESULT hr = g_dxgiSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
		HR_ASSERT_SUCCESS(hr, "Failed to resize swap chain");

		CreateBackBufferRenderTargetView();

		SetViewport(newSize);
	}

	void URenderer::Frame(float framePercent)
	{
		(void)framePercent;

		BeginFrame();
		Draw();
		EndFrame();
	}

	void URenderer::BeginFrame()
	{
		static const FLOAT clearColor[] = { 0.392f, 0.584f, 0.929f, 1.0f };
		g_d3dDeviceContext->ClearRenderTargetView(g_d3dBackBufferRenderTarget.Get(), clearColor);
	}

	void URenderer::Draw()
	{
		
	}

	void URenderer::EndFrame()
	{
		g_dxgiSwapChain->Present(0, 0);
		//g_dxgiSwapChain->Present1(0, 0, nullptr);
	}

	ComPtr<ID3D11ShaderResourceView> URenderer::CreateTextureFromFile(const eastl::string& inPath, uint64_t& outWidth, uint64_t& outHeight) const
	{
		auto widePath = utf8util::UTF16FromUTF8(inPath);
		auto extension = inPath.substr(inPath.find_last_of('.'));
		extension.make_lower();

		ComPtr<ID3D11ShaderResourceView> srv;
		ComPtr<ID3D11Resource> texture;

		HRESULT hr;
		if (extension == ".dds")
		{
			hr = DirectX::CreateDDSTextureFromFile(g_d3dDevice.Get(), widePath.c_str(), texture.GetAddressOf(), srv.GetAddressOf());
		}
		else if (extension == ".png" || extension == ".bmp")
		{
			hr = DirectX::CreateWICTextureFromFile(g_d3dDevice.Get(), widePath.c_str(), texture.GetAddressOf(), srv.GetAddressOf());
		}
		else
		{
			LOG(LogTextureImport, Error, "Can only load textures of type DDS, PNG, or BMP\n");
			return nullptr;
		}

		if (FAILED(hr))
		{
			wchar_t* err = nullptr;
			if (!FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_MAX_WIDTH_MASK,
								nullptr,
								hr,
								MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
								reinterpret_cast<LPTSTR>(&err),
								0,
								nullptr))
			{
				LOG(LogTextureImport, Error, "Unknown error creating texture from file: [0x%08x]\n", hr);
				return nullptr;
			}

			LOG(LogTextureImport, Error, "Error creating texture from file: [0x%08x] %ls\n", hr, err);
			LocalFree(err);
			return nullptr;
		}

		CD3D11_TEXTURE2D_DESC textureDesc;
		static_cast<ID3D11Texture2D*>(texture.Get())->GetDesc(&textureDesc);
		outWidth = textureDesc.Width;
		outHeight = textureDesc.Height;

		return srv;
	}

	ComPtr<ID3D11Buffer> URenderer::CreateBuffer(const void* inData, UINT inDataSize, D3D11_USAGE inUsage, UINT inBindFlags, UINT inCpuAccessFlags) const
	{
		ComPtr<ID3D11Buffer> buffer;

		D3D11_BUFFER_DESC bufferDesc;
		bufferDesc.Usage = inUsage;
		bufferDesc.ByteWidth = inDataSize;
		bufferDesc.BindFlags = inBindFlags;
		bufferDesc.CPUAccessFlags = inCpuAccessFlags;
		bufferDesc.MiscFlags = 0;
		bufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA initialData;
		initialData.pSysMem = inData;
		initialData.SysMemPitch = 0;
		initialData.SysMemSlicePitch = 0;

		HRESULT hr = g_d3dDevice->CreateBuffer(&bufferDesc, inData ? &initialData : nullptr, buffer.GetAddressOf());
		HR_ASSERT_SUCCESS(hr, "Failed to create graphics buffer");

		return buffer;
	}

	ComPtr<ID3D11Buffer> URenderer::CreateVertexBuffer(const void* inData, UINT inDataSize) const
	{
		MAD_ASSERT_DESC(inData != nullptr, "Must specify initial vertex data");
		return CreateBuffer(inData, inDataSize, D3D11_USAGE_IMMUTABLE, D3D11_BIND_VERTEX_BUFFER, 0);
	}

	ComPtr<ID3D11Buffer> URenderer::CreateIndexBuffer(const void* inData, UINT inDataSize) const
	{
		MAD_ASSERT_DESC(inData != nullptr, "Must specify initial index data");
		return CreateBuffer(inData, inDataSize, D3D11_USAGE_IMMUTABLE, D3D11_BIND_INDEX_BUFFER, 0);
	}

	ComPtr<ID3D11Buffer> URenderer::CreateConstantBuffer(const void* inData, UINT inDataSize) const
	{
		MAD_ASSERT_DESC(inDataSize % 16 == 0, "Constant buffer size must be evenly divisible by 16");
		return CreateBuffer(inData, inDataSize, D3D11_USAGE_DYNAMIC, D3D11_BIND_CONSTANT_BUFFER, D3D11_CPU_ACCESS_WRITE);
	}

	inline void* URenderer::MapBuffer(ComPtr<ID3D11Buffer> inBuffer) const
	{
		D3D11_MAPPED_SUBRESOURCE subResource;
		g_d3dDeviceContext->Map(inBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
		return subResource.pData;
	}

	inline void URenderer::UnmapBuffer(ComPtr<ID3D11Buffer> inBuffer) const
	{
		g_d3dDeviceContext->PSSetConstantBuffers(0, 0, nullptr);
		g_d3dDeviceContext->PSSetConstantBuffers1(0, 0, nullptr, nullptr, nullptr);
		g_d3dDeviceContext->Unmap(inBuffer.Get(), 0);
	}

	inline void URenderer::UpdateBuffer(ComPtr<ID3D11Buffer> inBuffer, const void* inData, size_t inDataSize) const
	{
		auto data = MapBuffer(inBuffer);
		memcpy(data, inData, inDataSize);
		UnmapBuffer(inBuffer);
	}

	inline void URenderer::SetViewport(float inWidth, float inHeight) const
	{
		D3D11_VIEWPORT vp;
		vp.TopLeftX = 0.0f;
		vp.TopLeftY = 0.0f;
		vp.Width = inWidth;
		vp.Height = inHeight;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;

		g_d3dDeviceContext->RSSetViewports(1, &vp);
	}

	inline void URenderer::SetViewport(POINT inDimensions) const
	{
		SetViewport(static_cast<float>(inDimensions.x), static_cast<float>(inDimensions.y));
	}

	void URenderer::SetFullScreen(bool inIsFullscreen) const
	{
		HRESULT hr = g_dxgiSwapChain->SetFullscreenState(inIsFullscreen, nullptr);
		HR_ASSERT_SUCCESS(hr, "Failed to set fullscreen state");
	}
}
