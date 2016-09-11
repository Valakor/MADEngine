#include "Rendering/GraphicsDriver.h"
#include <d3dcompiler.h>

#include <DirectXTK/DDSTextureLoader.h>
#include <DirectXTK/WICTextureLoader.h>
#include <wrl/client.h>

#include <EASTL/hash_map.h>
#include <EASTL/string.h>
#include <EASTL/algorithm.h>

#include "Core/GameWindow.h"
#include "Misc/Assert.h"
#include "Misc/Logging.h"
#include "Misc/utf8conv.h"

using Microsoft::WRL::ComPtr;

namespace MAD
{
	DECLARE_LOG_CATEGORY(LogGraphicsDevice);
	DECLARE_LOG_CATEGORY(LogTextureImport);

#ifdef _DEBUG
#define HR_ASSERT_SUCCESS(hr, desc) MAD_ASSERT_DESC(SUCCEEDED(hr), desc)

#define ID_ASSERT_VALID(id, cache, desc)		\
	MAD_ASSERT_DESC((id).IsValid(), desc);		\
	MAD_ASSERT_DESC(cache.count(id) == 1, desc)

#define ID_GET_SAFE(outVar, id, cache, desc)	\
	ID_ASSERT_VALID(id, cache, desc);			\
	auto outVar = cache[id]
#else
#define HR_ASSERT_SUCCESS(hr, desc) (void)(hr)
#define ID_ASSERT_VALID(id, cache, desc) (void)0
#define ID_GET_SAFE(outVar, id, cache, desc) auto outVar = cache[id]
#endif

#define ID_TRY_GET(outVar, id, cache, desc)		\
	eastl::remove_reference<decltype(cache[id])>::type outVar = nullptr;	\
	if ((id).IsValid())							\
	{											\
		ID_ASSERT_VALID(id, cache, desc);		\
		outVar = cache[id];						\
	}

#define ID_DESTROY(id, cache)					\
		if (id.IsValid())						\
		{										\
			auto iter = cache.find(id);			\
			if (iter != cache.end())			\
			{									\
				cache.erase(iter);				\
			}									\
		}										\
		id.Invalidate()

#define MEM_ZERO(s) memset(&s, 0, sizeof(s))

	namespace
	{
		HWND							g_nativeWindowHandle = nullptr;

		ComPtr<ID3D11Device2>			g_d3dDevice;
		ComPtr<ID3D11DeviceContext2>	g_d3dDeviceContext;
		ComPtr<IDXGISwapChain2>			g_dxgiSwapChain;

		// Constant configuration
		const UINT g_swapChainBufferCount = 3;
		const DXGI_FORMAT g_swapChainFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

		// Graphics driver object stores
#define DECLARE_OBJECT_STORE(IdType, D3DType, storeName) eastl::hash_map<IdType, ComPtr<D3DType>> storeName;

		DECLARE_OBJECT_STORE(SVertexShaderId, ID3D11VertexShader, g_vertexShaderStore);
		DECLARE_OBJECT_STORE(SPixelShaderId, ID3D11PixelShader, g_pixelShaderStore);
		DECLARE_OBJECT_STORE(SInputLayoutId, ID3D11InputLayout, g_inputLayoutStore);
		DECLARE_OBJECT_STORE(SRenderTargetId, ID3D11RenderTargetView, g_renderTargetStore);
		DECLARE_OBJECT_STORE(SDepthStencilId, ID3D11DepthStencilView, g_depthStencilStore);
		DECLARE_OBJECT_STORE(SDepthStencilStateId, ID3D11DepthStencilState, g_depthStencilStateStore);
		DECLARE_OBJECT_STORE(SBlendStateId, ID3D11BlendState, g_blendStateStore);
		DECLARE_OBJECT_STORE(SSamplerStateId, ID3D11SamplerState, g_samplerStateStore);
		DECLARE_OBJECT_STORE(SRasterizerStateId, ID3D11RasterizerState1, g_rasterizerStateStore);
		DECLARE_OBJECT_STORE(SShaderResourceId, ID3D11ShaderResourceView, g_shaderResourceViewStore);
		DECLARE_OBJECT_STORE(SBufferId, ID3D11Buffer, g_bufferStore);

#undef DECLARE_OBJECT_STORE

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

		void CreateSwapChain(HWND inWindow)
		{
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

			DXGI_SWAP_CHAIN_DESC1 scd;
			MEM_ZERO(scd);
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

			DXGI_SWAP_CHAIN_FULLSCREEN_DESC scfd;
			MEM_ZERO(scfd);
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
		}
	}

	UGraphicsDriver::UGraphicsDriver() { }

	void UGraphicsDriver::CreateBackBufferRenderTargetView()
	{
		ComPtr<ID3D11Texture2D> backBuffer;
		HRESULT hr = g_dxgiSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf()));
		HR_ASSERT_SUCCESS(hr, "Failed to get back buffer from swap chain");

		if (m_backBuffer.IsValid())
		{
			// Need to release the old one
			ID_ASSERT_VALID(m_backBuffer, g_renderTargetStore, "");
			g_renderTargetStore[m_backBuffer].Reset();
		}
		else
		{
			m_backBuffer = SRenderTargetId::Next();
		}

		ComPtr<ID3D11RenderTargetView>& backBufferRTV = g_renderTargetStore[m_backBuffer];

		hr = g_d3dDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, backBufferRTV.GetAddressOf());
		HR_ASSERT_SUCCESS(hr, "Failed to create render target view from back buffer");
	}

	SInputLayoutId UGraphicsDriver::ReflectInputLayout(ID3DBlob* inTargetBlob) const
	{
		// Reflect shader info
		ID3D11ShaderReflection* pVertexShaderReflection = NULL;

		HRESULT hr = D3DReflect(inTargetBlob->GetBufferPointer(), inTargetBlob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&pVertexShaderReflection);
		HR_ASSERT_SUCCESS(hr, "Failed to reflect on the shader");
		
		// Get shader info
		D3D11_SHADER_DESC shaderDesc;
		pVertexShaderReflection->GetDesc(&shaderDesc);

		// Read input layout description from shader info
		eastl::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc;
		for (uint32_t i = 0; i < shaderDesc.InputParameters; i++)
		{
			D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
			pVertexShaderReflection->GetInputParameterDesc(i, &paramDesc);

			// fill out input element desc
			D3D11_INPUT_ELEMENT_DESC elementDesc;
			elementDesc.SemanticName = paramDesc.SemanticName;
			elementDesc.SemanticIndex = paramDesc.SemanticIndex;
			elementDesc.InputSlot = 0;
			elementDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			elementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			elementDesc.InstanceDataStepRate = 0;

			// determine DXGI format
			if (paramDesc.Mask == 1)
			{
				if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32_UINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32_SINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32_FLOAT;
			}
			else if (paramDesc.Mask <= 3)
			{
				if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32_UINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32_SINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
			}
			else if (paramDesc.Mask <= 7)
			{
				if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
			}
			else if (paramDesc.Mask <= 15)
			{
				if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			}

			//save element desc
			inputLayoutDesc.push_back(elementDesc);
		}

		// Create new Input Layout
		SInputLayoutId inputLayoutId = CreateInputLayout(&inputLayoutDesc[0], static_cast<UINT>(inputLayoutDesc.size()), inTargetBlob->GetBufferPointer(), inTargetBlob->GetBufferSize());

		//Free allocation shader reflection memory
		pVertexShaderReflection->Release();
		
		return inputLayoutId;
	}

	bool UGraphicsDriver::Init(UGameWindow& inWindow)
	{
		LOG(LogGraphicsDevice, Log, "Graphics driver initialization begin...\n");

		g_nativeWindowHandle = inWindow.GetHWnd();

		CreateDevice();
		CreateSwapChain(g_nativeWindowHandle);
		CreateBackBufferRenderTargetView();

		// Initialize our constant buffers
		m_constantBuffers.resize(AsIntegral(EConstantBufferSlot::MAX));
		m_constantBuffers[AsIntegral(EConstantBufferSlot::PerScene)] = CreateConstantBuffer(nullptr, sizeof(SPerSceneConstants));
		m_constantBuffers[AsIntegral(EConstantBufferSlot::PerFrame)] = CreateConstantBuffer(nullptr, sizeof(SPerFrameConstants));
		m_constantBuffers[AsIntegral(EConstantBufferSlot::PerPointLight)] = CreateConstantBuffer(nullptr, sizeof(SPerPointLightConstants));
		m_constantBuffers[AsIntegral(EConstantBufferSlot::PerDirectionalLight)] = CreateConstantBuffer(nullptr, sizeof(SPerDirectionalLightConstants));
		m_constantBuffers[AsIntegral(EConstantBufferSlot::PerMaterial)] = CreateConstantBuffer(nullptr, sizeof(SPerMaterialConstants));
		m_constantBuffers[AsIntegral(EConstantBufferSlot::PerDraw)] = CreateConstantBuffer(nullptr, sizeof(SPerDrawConstants));

		// Initialize our samplers
		m_samplers.resize(AsIntegral(ESamplerSlot::MAX));
		m_samplers[AsIntegral(ESamplerSlot::Point)] = CreateSamplerState();
		m_samplers[AsIntegral(ESamplerSlot::Linear)] = CreateSamplerState();
		m_samplers[AsIntegral(ESamplerSlot::Trilinear)] = CreateSamplerState();
		m_samplers[AsIntegral(ESamplerSlot::Anisotropic)] = CreateSamplerState();

		LOG(LogGraphicsDevice, Log, "Graphics driver initialization successful\n");
		return true;
	}

	void UGraphicsDriver::Shutdown()
	{
		if (g_d3dDeviceContext)
		{
			g_d3dDeviceContext->ClearState();
			g_d3dDeviceContext->Flush();
		}

		g_dxgiSwapChain->SetFullscreenState(FALSE, nullptr);

		g_dxgiSwapChain.Reset();
		g_d3dDeviceContext.Reset();
		g_d3dDevice.Reset();
	}

	void UGraphicsDriver::OnScreenSizeChanged()
	{
		if (!g_d3dDeviceContext) return;

		g_d3dDeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
		
		ID_ASSERT_VALID(m_backBuffer, g_renderTargetStore, "Back buffer should be valid if the device has been created");
		g_renderTargetStore[m_backBuffer].Reset();
		
		g_d3dDeviceContext->Flush();
		g_d3dDeviceContext->ClearState();

		HRESULT hr = g_dxgiSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
		HR_ASSERT_SUCCESS(hr, "Failed to resize swap chain");

		CreateBackBufferRenderTargetView();
	}

	SShaderResourceId UGraphicsDriver::CreateTextureFromFile(const eastl::string& inPath, uint64_t& outWidth, uint64_t& outHeight) const
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
			return SShaderResourceId::Invalid;
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
				return SShaderResourceId::Invalid;
			}

			LOG(LogTextureImport, Error, "Error creating texture from file: [0x%08x] %ls\n", hr, err);
			LocalFree(err);
			return SShaderResourceId::Invalid;
		}

		CD3D11_TEXTURE2D_DESC textureDesc;
		MEM_ZERO(textureDesc);
		static_cast<ID3D11Texture2D*>(texture.Get())->GetDesc(&textureDesc);
		outWidth = textureDesc.Width;
		outHeight = textureDesc.Height;

		auto shaderResourceViewId = SShaderResourceId::Next();
		g_shaderResourceViewStore.insert({ shaderResourceViewId, srv });
		return shaderResourceViewId;
	}

	bool UGraphicsDriver::CompileShaderFromFile(const eastl::string& inFileName, const eastl::string& inShaderEntryPoint, const eastl::string& inShaderModel, eastl::vector<char>& inOutCompileByteCode, SInputLayoutId* outOptInputLayout)
	{
		HRESULT hr = S_OK;

		DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
		// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
		// Setting this flag improves the shader debugging experience, but still allows 
		// the shaders to be optimized and to run exactly the way they will run in 
		// the release configuration of this program.
		dwShaderFlags |= D3DCOMPILE_DEBUG;

		// Disable optimizations to further improve shader debugging
		dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

		const size_t cSize = inFileName.length() + 1;
		size_t retCount;
		std::wstring wc(cSize, L'#');
		mbstowcs_s(&retCount, &wc[0], cSize, inFileName.c_str(), _TRUNCATE);

		ID3DBlob* pErrorBlob = nullptr;
		ID3DBlob* pBlobOut = nullptr;
		hr = D3DCompileFromFile(wc.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, inShaderEntryPoint.c_str(), inShaderModel.c_str(),
			dwShaderFlags, 0, &pBlobOut, &pErrorBlob);
		if (FAILED(hr))
		{
			if (pErrorBlob)
			{
				static wchar_t szBuffer[4096];
				_snwprintf_s(szBuffer, 4096, _TRUNCATE,
					L"%hs",
					(char*)pErrorBlob->GetBufferPointer());
				OutputDebugString(szBuffer);
				MessageBox(nullptr, szBuffer, L"Error", MB_OK);
				pErrorBlob->Release();
				MAD_ASSERT_DESC(hr == S_OK, "Shader Compilation Failed");
			}
			return false;
		}
		if (pErrorBlob)
		{
			pErrorBlob->Release();
		}

		//now copy to vector if we like it...
		if (pBlobOut)
		{
			// Before releasing the blob, reflect the input layout from the shader (if vertex shader)
			if (inShaderModel.substr(0, 2) == "vs" && outOptInputLayout)
			{
				*outOptInputLayout = ReflectInputLayout(pBlobOut);
			}

			size_t compiledCodeSize = pBlobOut->GetBufferSize();
			inOutCompileByteCode.resize(compiledCodeSize);
			std::memcpy(inOutCompileByteCode.data(), pBlobOut->GetBufferPointer(), compiledCodeSize);

			pBlobOut->Release();
		}

		return hr == S_OK;
	}

	// TODO: Potentially create macro for this (??)
	SVertexShaderId UGraphicsDriver::CreateVertexShader(const eastl::vector<char>& inCompiledVSByteCode)
	{
		ComPtr<ID3D11VertexShader> vertexShaderPtr;

		HRESULT hr = g_d3dDevice->CreateVertexShader(inCompiledVSByteCode.data(), inCompiledVSByteCode.size(), nullptr, vertexShaderPtr.GetAddressOf());

		MAD_ASSERT_DESC(hr == S_OK, "Failure Creating Vertex Shader From Compiled Shader Code");

		SVertexShaderId vertexShaderId;

		if (hr == S_OK)
		{
			vertexShaderId = SVertexShaderId::Next();

			g_vertexShaderStore.insert({ vertexShaderId, vertexShaderPtr });
		}
		
		return vertexShaderId;
	}

	SPixelShaderId UGraphicsDriver::CreatePixelShader(const eastl::vector<char>& inCompiledPSByteCode)
	{
		ComPtr<ID3D11PixelShader> pixelShaderPtr;

		HRESULT hr = g_d3dDevice->CreatePixelShader(inCompiledPSByteCode.data(), inCompiledPSByteCode.size(), nullptr, pixelShaderPtr.GetAddressOf());

		MAD_ASSERT_DESC(hr == S_OK, "Failure Creating Pixel Shader from Compiled Shader Code");

		SPixelShaderId pixelShaderId;

		if (hr == S_OK)
		{
			pixelShaderId = SPixelShaderId::Next();

			g_pixelShaderStore.insert({ pixelShaderId, pixelShaderPtr });
		}

		return pixelShaderId;
	}

	SRenderTargetId UGraphicsDriver::CreateRenderTarget(UINT inWidth, UINT inHeight, DXGI_FORMAT inFormat, SShaderResourceId* outOptionalShaderResource) const
	{
		D3D11_TEXTURE2D_DESC backingTexDesc;
		MEM_ZERO(backingTexDesc);
		backingTexDesc.Width = inWidth;
		backingTexDesc.Height = inHeight;
		backingTexDesc.MipLevels = 1;
		backingTexDesc.ArraySize = 1;
		backingTexDesc.Format = inFormat;
		backingTexDesc.SampleDesc.Count = 1;
		backingTexDesc.SampleDesc.Quality = 0;
		backingTexDesc.Usage = D3D11_USAGE_DEFAULT;
		backingTexDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
		backingTexDesc.CPUAccessFlags = 0;
		backingTexDesc.MiscFlags = 0;

		if (outOptionalShaderResource)
		{
			backingTexDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
		}
		
		ComPtr<ID3D11Texture2D> backingTex;
		HRESULT hr = g_d3dDevice->CreateTexture2D(&backingTexDesc, nullptr, backingTex.GetAddressOf());
		HR_ASSERT_SUCCESS(hr, "Failed to create backing texture for Render Target");

		D3D11_RENDER_TARGET_VIEW_DESC renderTargetDesc;
		MEM_ZERO(renderTargetDesc);
		renderTargetDesc.Format = backingTexDesc.Format;
		renderTargetDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		renderTargetDesc.Texture2D.MipSlice = 0;

		ComPtr<ID3D11RenderTargetView> renderTarget;
		hr = g_d3dDevice->CreateRenderTargetView(backingTex.Get(), &renderTargetDesc, renderTarget.GetAddressOf());
		HR_ASSERT_SUCCESS(hr, "Failed to create render target from backing texture");

		if (outOptionalShaderResource)
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceDesc;
			MEM_ZERO(shaderResourceDesc);
			shaderResourceDesc.Format = backingTexDesc.Format;
			shaderResourceDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			shaderResourceDesc.Texture2D.MostDetailedMip = 0;
			shaderResourceDesc.Texture2D.MipLevels = 1;

			ComPtr<ID3D11ShaderResourceView> shaderResource;
			hr = g_d3dDevice->CreateShaderResourceView(backingTex.Get(), &shaderResourceDesc, shaderResource.GetAddressOf());
			HR_ASSERT_SUCCESS(hr, "Failed to create shader resource view from backing texture");

			auto shaderResourceId = SShaderResourceId::Next();
			g_shaderResourceViewStore.insert({ shaderResourceId, shaderResource });
			*outOptionalShaderResource = shaderResourceId;
		}
		
		auto renderTargetId = SRenderTargetId::Next();
		g_renderTargetStore.insert({ renderTargetId, renderTarget });
		return renderTargetId;
	}

	SInputLayoutId UGraphicsDriver::CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* inElements, UINT inNumElements, const eastl::vector<char>& inCompiledVertexShader) const
	{
		return CreateInputLayout(inElements, inNumElements, inCompiledVertexShader.data(), inCompiledVertexShader.size());
	}

	SInputLayoutId UGraphicsDriver::CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* inElements, UINT inNumElements, const void* inCompiledVSByteCode, size_t inByteCodeSize) const
	{
		ComPtr<ID3D11InputLayout> inputLayout;
		HRESULT hr = g_d3dDevice->CreateInputLayout(inElements, inNumElements, inCompiledVSByteCode, inByteCodeSize, inputLayout.GetAddressOf());
		HR_ASSERT_SUCCESS(hr, "Failed creating input layout");

		auto inputLayoutId = SInputLayoutId::Next();
		g_inputLayoutStore.insert({ inputLayoutId, inputLayout });
		return inputLayoutId;
	}

	SSamplerStateId UGraphicsDriver::CreateSamplerState() const
	{
		D3D11_SAMPLER_DESC samplerDesc;
		MEM_ZERO(samplerDesc);
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.MipLODBias = 0;
		samplerDesc.MaxAnisotropy = 0;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
		
		ComPtr<ID3D11SamplerState> samplerState;
		HRESULT hr = g_d3dDevice->CreateSamplerState(&samplerDesc, samplerState.GetAddressOf());
		HR_ASSERT_SUCCESS(hr, "Failed to create sampler state");

		auto samplerStateId = SSamplerStateId::Next();
		g_samplerStateStore.insert({ samplerStateId, samplerState });
		return samplerStateId;
	}

	SDepthStencilId UGraphicsDriver::CreateDepthStencil(int inWidth, int inHeight, SShaderResourceId* outOptionalShaderResource) const
	{
		D3D11_TEXTURE2D_DESC backingTexDesc;
		MEM_ZERO(backingTexDesc);
		backingTexDesc.Width = inWidth;
		backingTexDesc.Height = inHeight;
		backingTexDesc.MipLevels = 1;
		backingTexDesc.ArraySize = 1;
		backingTexDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
		backingTexDesc.SampleDesc.Count = 1;
		backingTexDesc.SampleDesc.Quality = 0;
		backingTexDesc.Usage = D3D11_USAGE_DEFAULT;
		backingTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		backingTexDesc.CPUAccessFlags = 0;
		backingTexDesc.MiscFlags = 0;

		if (outOptionalShaderResource)
		{
			backingTexDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
		}

		ComPtr<ID3D11Texture2D> backingTex;
		HRESULT hr = g_d3dDevice->CreateTexture2D(&backingTexDesc, nullptr, backingTex.GetAddressOf());
		HR_ASSERT_SUCCESS(hr, "Failed to create backing texture for depth stencil");

		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilDesc;
		MEM_ZERO(depthStencilDesc);
		depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilDesc.Texture2D.MipSlice = 0;

		ComPtr<ID3D11DepthStencilView> depthStencil;
		hr = g_d3dDevice->CreateDepthStencilView(backingTex.Get(), &depthStencilDesc, depthStencil.GetAddressOf());
		HR_ASSERT_SUCCESS(hr, "Failed to create depth stencil view from backing texture");

		if (outOptionalShaderResource)
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceDesc;
			MEM_ZERO(shaderResourceDesc);
			shaderResourceDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			shaderResourceDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			shaderResourceDesc.Texture2D.MostDetailedMip = 0;
			shaderResourceDesc.Texture2D.MipLevels = 1;

			ComPtr<ID3D11ShaderResourceView> shaderResource;
			hr = g_d3dDevice->CreateShaderResourceView(backingTex.Get(), &shaderResourceDesc, shaderResource.GetAddressOf());
			HR_ASSERT_SUCCESS(hr, "Failed to create shader resource view from backing texture");

			auto shaderResourceId = SShaderResourceId::Next();
			g_shaderResourceViewStore.insert({ shaderResourceId, shaderResource });
			*outOptionalShaderResource = shaderResourceId;
		}

		auto depthStencilId = SDepthStencilId::Next();
		g_depthStencilStore.insert({ depthStencilId, depthStencil });
		return depthStencilId;
	}

	SDepthStencilStateId UGraphicsDriver::CreateDepthStencilState(bool inDepthTestEnable, D3D11_COMPARISON_FUNC inComparisonFunc) const
	{
		D3D11_DEPTH_STENCIL_DESC stateDesc;
		MEM_ZERO(stateDesc);

		// Depth test parameters
		stateDesc.DepthEnable = inDepthTestEnable;
		stateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		stateDesc.DepthFunc = inComparisonFunc;

		// Stencil test parameters
		stateDesc.StencilEnable = false;
		stateDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
		stateDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

		// Stencil operations if pixel is front-facing
		stateDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		stateDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
		stateDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		stateDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		// Stencil operations if pixel is back-facing
		stateDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		stateDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
		stateDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		stateDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		ComPtr<ID3D11DepthStencilState> depthStencilState;
		HRESULT hr = g_d3dDevice->CreateDepthStencilState(&stateDesc, depthStencilState.GetAddressOf());
		HR_ASSERT_SUCCESS(hr, "Failed to create depth stencil state");

		auto depthStencilStateId = SDepthStencilStateId::Next();
		g_depthStencilStateStore.insert({ depthStencilStateId, depthStencilState });
		return depthStencilStateId;
	}

	SRasterizerStateId UGraphicsDriver::CreateRasterizerState(D3D11_FILL_MODE inFillMode, D3D11_CULL_MODE inCullMode) const
	{
		D3D11_RASTERIZER_DESC1 rasterDesc;
		MEM_ZERO(rasterDesc);
		rasterDesc.FillMode = inFillMode;
		rasterDesc.CullMode = inCullMode;
		rasterDesc.FrontCounterClockwise = false;

		ComPtr<ID3D11RasterizerState1> raster;
		HRESULT hr = g_d3dDevice->CreateRasterizerState1(&rasterDesc, raster.GetAddressOf());
		HR_ASSERT_SUCCESS(hr, "Failed to create rasterizer state");

		auto rasterId = SRasterizerStateId::Next();
		g_rasterizerStateStore.insert({ rasterId, raster });
		return rasterId;
	}

	SBlendStateId UGraphicsDriver::CreateBlendState(bool inEnableBlend) const
	{
		D3D11_BLEND_DESC1 blendDesc;
		MEM_ZERO(blendDesc);
		blendDesc.AlphaToCoverageEnable = FALSE;
		blendDesc.IndependentBlendEnable = FALSE;
		blendDesc.RenderTarget[0].BlendEnable = inEnableBlend;
		blendDesc.RenderTarget[0].LogicOpEnable = FALSE;
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].LogicOp = D3D11_LOGIC_OP_NOOP;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		ComPtr<ID3D11BlendState1> blendState;
		HRESULT hr = g_d3dDevice->CreateBlendState1(&blendDesc, blendState.GetAddressOf());
		HR_ASSERT_SUCCESS(hr, "Failed to create blend state");

		auto blendStateId = SBlendStateId::Next();
		g_blendStateStore.insert({ blendStateId, blendState });
		return blendStateId;
	}

	SBufferId UGraphicsDriver::CreateBuffer(const void* inData, UINT inDataSize, D3D11_USAGE inUsage, UINT inBindFlags, UINT inCpuAccessFlags) const
	{
		D3D11_BUFFER_DESC bufferDesc;
		MEM_ZERO(bufferDesc);
		bufferDesc.Usage = inUsage;
		bufferDesc.ByteWidth = inDataSize;
		bufferDesc.BindFlags = inBindFlags;
		bufferDesc.CPUAccessFlags = inCpuAccessFlags;
		bufferDesc.MiscFlags = 0;
		bufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA initialData;
		MEM_ZERO(initialData);
		initialData.pSysMem = inData;
		initialData.SysMemPitch = 0;
		initialData.SysMemSlicePitch = 0;

		ComPtr<ID3D11Buffer> buffer;
		HRESULT hr = g_d3dDevice->CreateBuffer(&bufferDesc, inData ? &initialData : nullptr, buffer.GetAddressOf());
		HR_ASSERT_SUCCESS(hr, "Failed to create graphics buffer");

		auto bufferId = SBufferId::Next();
		g_bufferStore.insert({ bufferId, buffer });
		return bufferId;
	}

	SBufferId UGraphicsDriver::CreateVertexBuffer(const void* inData, UINT inDataSize) const
	{
		MAD_ASSERT_DESC(inData != nullptr, "Must specify initial vertex data");
		return CreateBuffer(inData, inDataSize, D3D11_USAGE_IMMUTABLE, D3D11_BIND_VERTEX_BUFFER, 0);
	}

	SBufferId UGraphicsDriver::CreateIndexBuffer(const void* inData, UINT inDataSize) const
	{
		MAD_ASSERT_DESC(inData != nullptr, "Must specify initial index data");
		return CreateBuffer(inData, inDataSize, D3D11_USAGE_IMMUTABLE, D3D11_BIND_INDEX_BUFFER, 0);
	}

	SBufferId UGraphicsDriver::CreateConstantBuffer(const void* inData, UINT inDataSize) const
	{
		MAD_ASSERT_DESC(inDataSize % 16 == 0, "Constant buffer size must be evenly divisible by 16");
		return CreateBuffer(inData, inDataSize, D3D11_USAGE_DYNAMIC, D3D11_BIND_CONSTANT_BUFFER, D3D11_CPU_ACCESS_WRITE);
	}

	void* UGraphicsDriver::MapBuffer(SBufferId inBuffer) const
	{
		ID_GET_SAFE(buffer, inBuffer, g_bufferStore, "Invalid buffer");

		D3D11_MAPPED_SUBRESOURCE subResource;
		g_d3dDeviceContext->Map(buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
		return subResource.pData;
	}

	void UGraphicsDriver::UnmapBuffer(SBufferId inBuffer) const
	{
		ID_GET_SAFE(buffer, inBuffer, g_bufferStore, "Invalid buffer");
		g_d3dDeviceContext->Unmap(buffer.Get(), 0);
	}

	void UGraphicsDriver::UpdateBuffer(SBufferId inBuffer, const void* inData, size_t inDataSize) const
	{
		auto data = MapBuffer(inBuffer);
		memcpy(data, inData, inDataSize);
		UnmapBuffer(inBuffer);
	}

	void UGraphicsDriver::UpdateBuffer(EConstantBufferSlot inSlot, const void* inData, size_t inDataSize) const
	{
		MAD_ASSERT_DESC(AsIntegral(inSlot) < AsIntegral(EConstantBufferSlot::MAX), "Invalid EConstantBufferSlot");
		UpdateBuffer(m_constantBuffers[AsIntegral(inSlot)], inData, inDataSize);
	}

	void UGraphicsDriver::SetViewport(float inX, float inY, float inWidth, float inHeight) const
	{
		D3D11_VIEWPORT vp;
		vp.TopLeftX = inX;
		vp.TopLeftY = inY;
		vp.Width = inWidth;
		vp.Height = inHeight;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;

		g_d3dDeviceContext->RSSetViewports(1, &vp);
	}

	void UGraphicsDriver::SetViewport(int inX, int inY, int inWidth, int inHeight) const
	{
		SetViewport(static_cast<float>(inX), static_cast<float>(inY), static_cast<float>(inWidth), static_cast<float>(inHeight));
	}

	void UGraphicsDriver::SetRenderTargets(const SRenderTargetId* inRenderTargets, int inNumRenderTargets, const SDepthStencilId* inOptionalDepthStencil) const
	{
		MAD_ASSERT_DESC(inNumRenderTargets <= D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, "Cannot bind more than 8 render targets at once");

		ID3D11RenderTargetView* renderTargets[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] =
		{
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
		};

		for (int i = 0 ; i < inNumRenderTargets; ++i)
		{
			const auto& renderTargetId = inRenderTargets[i];
			ID_GET_SAFE(renderTarget, renderTargetId, g_renderTargetStore, "Invalid render target");
			renderTargets[i] = renderTarget.Get();
		}

		ID3D11DepthStencilView* depthStencil = nullptr;
		if (inOptionalDepthStencil)
		{
			ID_GET_SAFE(depthStencilPtr, *inOptionalDepthStencil, g_depthStencilStore, "Invalid depth stencil");
			depthStencil = depthStencilPtr.Get();
		}

		g_d3dDeviceContext->OMSetRenderTargets(inNumRenderTargets, renderTargets, depthStencil);
	}

	void UGraphicsDriver::SetDepthStencilState(SDepthStencilStateId inDepthStencilState, UINT inStencilRef) const
	{
		ID_GET_SAFE(depthStencilState, inDepthStencilState, g_depthStencilStateStore, "Invalid depth stencil state");
		g_d3dDeviceContext->OMSetDepthStencilState(depthStencilState.Get(), inStencilRef);
	}

	void UGraphicsDriver::SetInputLayout(SInputLayoutId inInputLayout) const
	{
		ID_GET_SAFE(inputLayout, inInputLayout, g_inputLayoutStore, "Invalid input layout");
		g_d3dDeviceContext->IASetInputLayout(inputLayout.Get());
	}

	void UGraphicsDriver::SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY inPrimitiveTopology) const
	{
		g_d3dDeviceContext->IASetPrimitiveTopology(inPrimitiveTopology);
	}

	void UGraphicsDriver::SetVertexBuffer(SBufferId inVertexBuffer, UINT inVertexSize, UINT inVertexOffset) const
	{
		ID_GET_SAFE(buffer, inVertexBuffer, g_bufferStore, "Invalid vertex buffer");
		UINT byteOffset = inVertexOffset * inVertexSize;
		g_d3dDeviceContext->IASetVertexBuffers(0, 1, buffer.GetAddressOf(), &inVertexSize, &byteOffset);
	}

	void UGraphicsDriver::SetIndexBuffer(SBufferId inIndexBuffer, UINT inIndexOffset) const
	{
		ID_GET_SAFE(buffer, inIndexBuffer, g_bufferStore, "Invalid index buffer");
		g_d3dDeviceContext->IASetIndexBuffer(buffer.Get(), DXGI_FORMAT_R16_UINT, 2 * inIndexOffset);
	}

	void UGraphicsDriver::SetVertexShader(SVertexShaderId inVertexShader) const
	{
		ID_GET_SAFE(shader, inVertexShader, g_vertexShaderStore, "Invalid vertex shader");
		g_d3dDeviceContext->VSSetShader(shader.Get(), nullptr, 0);
	}

	void UGraphicsDriver::SetVertexConstantBuffer(SBufferId inBuffer, UINT inSlot, UINT inOffset, UINT inLength) const
	{
		MAD_ASSERT_DESC(inOffset % 16 == 0, "Offset into constant buffer must be divisible by 16 (as it is measured in # of shader constants)");
		MAD_ASSERT_DESC(inLength % 16 == 0, "Length of used constant buffer must be divisible by 16 (as it is measured in # of shader constants)");

		inOffset /= 16;
		inLength /= 16;

		ID_GET_SAFE(buffer, inBuffer, g_bufferStore, "Invalid constant buffer");
		g_d3dDeviceContext->VSSetConstantBuffers1(inSlot, 1, buffer.GetAddressOf(), &inOffset, &inLength);
	}

	void UGraphicsDriver::SetPixelShader(SPixelShaderId inPixelShader) const
	{
		ID_GET_SAFE(shader, inPixelShader, g_pixelShaderStore, "Invalid pixel shader");
		g_d3dDeviceContext->PSSetShader(shader.Get(), nullptr, 0);
	}

	void UGraphicsDriver::SetPixelConstantBuffer(SBufferId inBuffer, UINT inSlot, UINT inOffset, UINT inLength) const
	{
		MAD_ASSERT_DESC(inOffset % 16 == 0, "Offset into constant buffer must be divisible by 16 (as it is measured in # of shader constants)");
		MAD_ASSERT_DESC(inLength % 16 == 0, "Length of used constant buffer must be divisible by 16 (as it is measured in # of shader constants)");

		inOffset /= 16;
		inLength /= 16;

		ID_GET_SAFE(buffer, inBuffer, g_bufferStore, "Invalid constant buffer");
		g_d3dDeviceContext->PSSetConstantBuffers1(inSlot, 1, buffer.GetAddressOf(), &inOffset, &inLength);
	}

	void UGraphicsDriver::SetPixelSamplerState(SSamplerStateId inSamplerState, UINT inSlot) const
	{
		ID_GET_SAFE(sampler, inSamplerState, g_samplerStateStore, "Invalid sampler state");
		g_d3dDeviceContext->PSSetSamplers(inSlot, 1, sampler.GetAddressOf());
	}

	void UGraphicsDriver::SetPixelShaderResource(SShaderResourceId inShaderResource, UINT inSlot) const
	{
		ID_TRY_GET(shaderResource, inShaderResource, g_shaderResourceViewStore, "Invalid shader resource");
		g_d3dDeviceContext->PSSetShaderResources(inSlot, 1, shaderResource.GetAddressOf());
	}

	void UGraphicsDriver::SetPixelShaderResource(SShaderResourceId inShaderResource, ETextureSlot inSlot) const
	{
		MAD_ASSERT_DESC(AsIntegral(inSlot) < AsIntegral(ETextureSlot::MAX), "Invalid ETextureSlot");
		SetPixelShaderResource(inShaderResource, AsIntegral(inSlot));
	}

	void UGraphicsDriver::SetRasterizerState(SRasterizerStateId inRasterizerState) const
	{
		ID_GET_SAFE(raster, inRasterizerState, g_rasterizerStateStore, "Invalid rasterizer state");
		g_d3dDeviceContext->RSSetState(raster.Get());
	}

	void UGraphicsDriver::SetBlendState(SBlendStateId inBlendstate) const
	{
		ID_TRY_GET(blend, inBlendstate, g_blendStateStore, "Invalid blend state");
		static FLOAT blendFactor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		g_d3dDeviceContext->OMSetBlendState(blend.Get(), blendFactor, 0xffffffff);
	}

	void UGraphicsDriver::DestroyDepthStencil(SDepthStencilId& inOutDepthStencil) const
	{
		ID_DESTROY(inOutDepthStencil, g_depthStencilStore);
	}

	void UGraphicsDriver::DestroyShaderResource(SShaderResourceId& inOutShaderResource) const
	{
		ID_DESTROY(inOutShaderResource, g_shaderResourceViewStore);
	}

	void UGraphicsDriver::DestroyRenderTarget(SRenderTargetId& inOutRenderTarget) const
	{
		ID_DESTROY(inOutRenderTarget, g_renderTargetStore);
	}

	void UGraphicsDriver::SetFullScreen(bool inIsFullscreen) const
	{
		HRESULT hr = g_dxgiSwapChain->SetFullscreenState(inIsFullscreen, nullptr);
		HR_ASSERT_SUCCESS(hr, "Failed to set fullscreen state");
	}

	void UGraphicsDriver::ClearBackBuffer(const float inColor[4])
	{
		ClearRenderTarget(m_backBuffer, inColor);
	}

	void UGraphicsDriver::ClearRenderTarget(SRenderTargetId inRenderTarget, const float inColor[4]) const
	{
		ID_GET_SAFE(renderTarget, inRenderTarget, g_renderTargetStore, "Invalid render target");
		g_d3dDeviceContext->ClearRenderTargetView(renderTarget.Get(), inColor);
	}

	void UGraphicsDriver::ClearDepthStencil(SDepthStencilId inDepthStencil, bool inClearDepth, float inDepth, bool inClearStencil, UINT8 inStencil) const
	{
		ID_GET_SAFE(depthStencil, inDepthStencil, g_depthStencilStore, "Invalid depth stencil view");
		
		UINT clearFlags = 0;
		if (inClearDepth) clearFlags |= D3D11_CLEAR_DEPTH;
		if (inClearStencil) clearFlags |= D3D11_CLEAR_STENCIL;

		g_d3dDeviceContext->ClearDepthStencilView(depthStencil.Get(), clearFlags, inDepth, inStencil);
	}

	void UGraphicsDriver::Draw(int inVertexCount, int inStartVertex) const
	{
		g_d3dDeviceContext->Draw(inVertexCount, inStartVertex);
	}

	void UGraphicsDriver::DrawIndexed(int inIndexCount, int inStartIndex, int inBaseVertex) const
	{
		g_d3dDeviceContext->DrawIndexed(inIndexCount, inStartIndex, inBaseVertex);
	}

	void UGraphicsDriver::Present() const
	{
		g_dxgiSwapChain->Present(0, 0);
	}
}
