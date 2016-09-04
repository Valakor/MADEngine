#include "Rendering/Renderer.h"

#define WIN32_LEAN_AND_MEAN
#include <d3d11_2.h>
#include <DirectXTK/DDSTextureLoader.h>
#include <DirectXTK/WICTextureLoader.h>
#include <wrl/client.h>

#include "Core/GameEngine.h"
#include "Core/GameWindow.h"
#include "Misc/Assert.h"
#include "Misc/Logging.h"
#include "Misc/utf8conv.h"

using Microsoft::WRL::ComPtr;

namespace MAD
{
	DECLARE_LOG_CATEGORY(LogRenderer);
	DECLARE_LOG_CATEGORY(LogTextureImport);

	namespace
	{
		ComPtr<ID3D11Device2>			g_d3dDevice;
		ComPtr<ID3D11DeviceContext2>	g_d3dDeviceContext;
		ComPtr<IDXGISwapChain2>			g_dxgiSwapChain;
		ComPtr<ID3D11RenderTargetView>	g_d3dBackBuffer;

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
			MAD_ASSERT_DESC(SUCCEEDED(hr), "Failed to create D3D device with feature level 11_1");

			g_d3dDevice.Reset();
			hr = d3dDevice0.As(&g_d3dDevice);
			MAD_ASSERT_DESC(SUCCEEDED(hr), "Failed to get device as D3d 11_2");

			g_d3dDeviceContext.Reset();
			hr = d3dDeviceContext0.As(&g_d3dDeviceContext);
			MAD_ASSERT_DESC(SUCCEEDED(hr), "Failed to get device context as D3d 11_2");
		}

		HRESULT CreateSwapChain(HWND inWindow, bool fullScreen)
		{
			(void)inWindow;

			HRESULT hr;

			ComPtr<IDXGIDevice3> dxgiDevice;
			hr = g_d3dDevice.As(&dxgiDevice);
			MAD_ASSERT_DESC(SUCCEEDED(hr), "Failed to get DXGI device from D3D device");

			ComPtr<IDXGIAdapter2> dxgiAdapter;
			hr = dxgiDevice->GetParent(__uuidof(IDXGIAdapter2), &dxgiAdapter);
			MAD_ASSERT_DESC(SUCCEEDED(hr), "Failed to get DXGI adapter from DXGI device");

			ComPtr<IDXGIFactory3> dxgiFactory;
			hr = dxgiAdapter->GetParent(_uuidof(IDXGIFactory3), &dxgiFactory);
			MAD_ASSERT_DESC(SUCCEEDED(hr), "Failed to get DXGI factory from DXGI adapter");

			DXGI_SWAP_CHAIN_DESC1 scd = { 0 };
			scd.Width = 0;
			scd.Height = 0;
			scd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			scd.Stereo = FALSE;
			scd.SampleDesc.Count = 1;
			scd.SampleDesc.Quality = 0;
			scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			scd.BufferCount = 2;
			scd.Scaling = DXGI_SCALING_NONE;
			scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
			scd.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
			scd.Flags = 0;

			DXGI_SWAP_CHAIN_FULLSCREEN_DESC scfd = { 0 };
			scfd.RefreshRate = { 0, 0 };
			scfd.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
			scfd.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
			scfd.Windowed = !fullScreen;

			ComPtr<IDXGISwapChain1> swapChain;
			hr = dxgiFactory->CreateSwapChainForHwnd(g_d3dDevice.Get(), inWindow, &scd, &scfd, nullptr, swapChain.ReleaseAndGetAddressOf());
			MAD_ASSERT_DESC(SUCCEEDED(hr), "Failed to create swap chain");

			g_dxgiSwapChain.Reset();
			hr = swapChain.As(&g_dxgiSwapChain);
			MAD_ASSERT_DESC(SUCCEEDED(hr), "Failed to get SwapChain1 as SwapChain2");

			return hr;
		}

		void CreateBackBufferRenderTargetView()
		{
			ComPtr<ID3D11Texture2D> backBuffer;
			HRESULT hr = g_dxgiSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf()));
			MAD_ASSERT_DESC(SUCCEEDED(hr), "Failed to get back buffer from swap chain");

			hr = g_d3dDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, g_d3dBackBuffer.ReleaseAndGetAddressOf());
			MAD_ASSERT_DESC(SUCCEEDED(hr), "Failed to create render target view from back buffer");
		}
	}

	URenderer::URenderer()
	{ }

	bool URenderer::Init()
	{
		LOG(LogRenderer, Log, "Renderer initialization begin...\n");

		CreateDevice();

		auto window = gEngine->GetWindow().GetHWnd();
		CreateSwapChain(window, false);

		CreateBackBufferRenderTargetView();

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

		g_d3dBackBuffer.Reset();
		g_dxgiSwapChain.Reset();
		g_d3dDeviceContext.Reset();
		g_d3dDevice.Reset();
	}

	void URenderer::OnScreenSizeChanged()
	{

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
		g_d3dDeviceContext->ClearRenderTargetView(g_d3dBackBuffer.Get(), clearColor);
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
}
