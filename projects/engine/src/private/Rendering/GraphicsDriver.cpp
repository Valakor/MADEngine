#include "Rendering/GraphicsDriver.h"

#include <INITGUID.H>
#include <D3DCompiler.h>

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
#include "Rendering/InputLayoutCache.h"

using Microsoft::WRL::ComPtr;

namespace MAD
{
	DECLARE_LOG_CATEGORY(LogGraphicsDevice);
	DECLARE_LOG_CATEGORY(LogTextureImport);

#define HR_CHECK(expr, desc) MAD_CHECK_DESC(SUCCEEDED(expr), desc)

#ifdef _DEBUG
#define ID_ASSERT_VALID(id, cache, desc)		\
	MAD_ASSERT_DESC((id).IsValid(), desc);		\
	MAD_ASSERT_DESC(cache.count(id) == 1, desc)

#define ID_GET_SAFE(outVar, id, cache, desc)	\
	ID_ASSERT_VALID(id, cache, desc);			\
	auto outVar = cache[id]
#else
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
	if (id.IsValid())							\
	{											\
		auto iter = cache.find(id);				\
		if (iter != cache.end())				\
		{										\
			cache.erase(iter);					\
		}										\
	}											\
	id.Invalidate()

#define MEM_ZERO(s) memset(&s, 0, sizeof(s))

	namespace
	{
		HWND								g_nativeWindowHandle = nullptr;

		ComPtr<ID3D11Device2>				g_d3dDevice;
		ComPtr<ID3D11DeviceContext2>		g_d3dDeviceContext;
		ComPtr<ID3DUserDefinedAnnotation>	g_d3dEvent;
		ComPtr<IDXGISwapChain2>				g_dxgiSwapChain;

		// Constant configuration
		const UINT g_swapChainBufferCount = 3;

		// Graphics driver object stores
#define DECLARE_OBJECT_STORE(IdType, D3DType, storeName) eastl::hash_map<IdType, ComPtr<D3DType>> storeName;

		//DECLARE_OBJECT_STORE(SVertexShaderId, ID3D11VertexShader, g_vertexShaderStore);
		//DECLARE_OBJECT_STORE(PixelShaderPtr_t, ID3D11PixelShader, g_pixelShaderStore);
		//DECLARE_OBJECT_STORE(InputLayoutPtr_t, ID3D11InputLayout, g_inputLayoutStore);
		//DECLARE_OBJECT_STORE(RenderTargetPtr_t, ID3D11RenderTargetView, g_renderTargetStore);
		//DECLARE_OBJECT_STORE(DepthStencilPtr_t, ID3D11DepthStencilView, g_depthStencilStore);
		//DECLARE_OBJECT_STORE(DepthStencilStatePtr_t, ID3D11DepthStencilState, g_depthStencilStateStore);
		//DECLARE_OBJECT_STORE(BlendStatePtr_t, ID3D11BlendState, g_blendStateStore);
		//DECLARE_OBJECT_STORE(SamplerStatePtr_t, ID3D11SamplerState, g_samplerStateStore);
		//DECLARE_OBJECT_STORE(RasterizerStatePtr_t, ID3D11RasterizerState1, g_rasterizerStateStore);
		DECLARE_OBJECT_STORE(SShaderResourceId, ID3D11ShaderResourceView, g_shaderResourceViewStore);
		DECLARE_OBJECT_STORE(SBufferId, ID3D11Buffer, g_bufferStore);

#undef DECLARE_OBJECT_STORE

		void CreateDevice()
		{
			UINT createDeviceFlags = D3D11_CREATE_DEVICE_SINGLETHREADED;
#ifdef _DEBUG
			createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

			// To make it easy on us, we're only going to support D3D 11.0 and above
			D3D_FEATURE_LEVEL featureLevels[] =
			{
				D3D_FEATURE_LEVEL_11_0,
			};
			UINT numFeatureLevels = ARRAYSIZE(featureLevels);

			D3D_FEATURE_LEVEL supportedFeatureLevel;
			ComPtr<ID3D11Device> d3dDevice0;
			ComPtr<ID3D11DeviceContext> d3dDeviceContext0;

			HR_CHECK(
				D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
					D3D11_SDK_VERSION, d3dDevice0.ReleaseAndGetAddressOf(), &supportedFeatureLevel, d3dDeviceContext0.ReleaseAndGetAddressOf()),
				"Failed to create D3D device with feature level 11_0");

			g_d3dDevice.Reset();
			HR_CHECK(d3dDevice0.As(&g_d3dDevice), "Failed to get device as D3d 11_2");

			g_d3dDeviceContext.Reset();
			HR_CHECK(d3dDeviceContext0.As(&g_d3dDeviceContext), "Failed to get device context as D3d 11_2");

#ifdef _DEBUG
			g_d3dEvent.Reset();
			HR_CHECK(g_d3dDeviceContext.As(&g_d3dEvent), "Failed to get debug event interface");
#endif
		}

		void CreateSwapChain(HWND inWindow)
		{
			ComPtr<IDXGIDevice3> dxgiDevice;
			HR_CHECK(g_d3dDevice.As(&dxgiDevice), "Failed to get DXGI device from D3D device");

			ComPtr<IDXGIAdapter2> dxgiAdapter;
			HR_CHECK(dxgiDevice->GetParent(__uuidof(IDXGIAdapter2), &dxgiAdapter), "Failed to get DXGI adapter from DXGI device");

			ComPtr<IDXGIFactory3> dxgiFactory;
			HR_CHECK(dxgiAdapter->GetParent(_uuidof(IDXGIFactory3), &dxgiFactory), "Failed to get DXGI factory from DXGI adapter");

			DXGI_SWAP_CHAIN_DESC1 scd;
			MEM_ZERO(scd);
			scd.Width = 0;
			scd.Height = 0;
			scd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
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
			HR_CHECK(dxgiFactory->CreateSwapChainForHwnd(g_d3dDevice.Get(), inWindow, &scd, &scfd, nullptr, swapChain.ReleaseAndGetAddressOf()), "Failed to create swap chain");

			g_dxgiSwapChain.Reset();
			HR_CHECK(swapChain.As(&g_dxgiSwapChain), "Failed to get SwapChain1 as SwapChain2");
		}
	}

	UGraphicsDriver::UGraphicsDriver() { }

	void UGraphicsDriver::CreateBackBufferRenderTargetView()
	{
		ComPtr<ID3D11Texture2D> backBuffer;
		HR_CHECK(g_dxgiSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf())), "Failed to get back buffer from swap chain");

		if (m_backBuffer)
		{
			// Need to release the old one
			m_backBuffer.Reset();
		}
		/*else
		{
			m_backBuffer = RenderTargetPtr_t::Next();
		}*/

		D3D11_RENDER_TARGET_VIEW_DESC renderTargetDesc;
		MEM_ZERO(renderTargetDesc);
		renderTargetDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		renderTargetDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		renderTargetDesc.Texture2D.MipSlice = 0;

		HR_CHECK(g_d3dDevice->CreateRenderTargetView(backBuffer.Get(), &renderTargetDesc, m_backBuffer.GetAddressOf()), "Failed to create render target view from back buffer");
	}

	void UGraphicsDriver::RegisterInputLayout(ID3DBlob* inTargetBlob)
	{
		// Reflect shader info
		ID3D11ShaderReflection* pVertexShaderReflection = nullptr;
		HR_CHECK(D3DReflect(inTargetBlob->GetBufferPointer(), inTargetBlob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&pVertexShaderReflection), "Failed to reflect on the shader");
		
		// Get shader info
		D3D11_SHADER_DESC shaderDesc;
		pVertexShaderReflection->GetDesc(&shaderDesc);

		// Read input layout description from shader info
		InputLayoutFlags_t inputLayoutFlags = 0;
		for (uint32_t i = 0; i < shaderDesc.InputParameters; i++)
		{
			D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
			pVertexShaderReflection->GetInputParameterDesc(i, &paramDesc);
			inputLayoutFlags |= UInputLayoutCache::GetFlagForSemanticName(paramDesc.SemanticName);
		}

		//Free allocation shader reflection memory
		pVertexShaderReflection->Release();

		// Register new Input Layout
		if (inputLayoutFlags != 0)
		{
			HR_CHECK(UInputLayoutCache::RegisterInputLayout(*this, inputLayoutFlags, inTargetBlob->GetBufferPointer(), inTargetBlob->GetBufferSize()),
				"Failed to register input layout reflected from VS shader");
		}
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
		for (unsigned i = 0; i < m_constantBuffers.size(); ++i)
		{
			SetVertexConstantBuffer(m_constantBuffers[i], i);
			SetPixelConstantBuffer(m_constantBuffers[i], i);
		}

#ifdef _DEBUG
		g_bufferStore[m_constantBuffers[AsIntegral(EConstantBufferSlot::PerScene)]]->SetPrivateData(WKPDID_D3DDebugObjectName, 8, "PerScene");
		g_bufferStore[m_constantBuffers[AsIntegral(EConstantBufferSlot::PerFrame)]]->SetPrivateData(WKPDID_D3DDebugObjectName, 8, "PerFrame");
		g_bufferStore[m_constantBuffers[AsIntegral(EConstantBufferSlot::PerPointLight)]]->SetPrivateData(WKPDID_D3DDebugObjectName, 13, "PerPointLight");
		g_bufferStore[m_constantBuffers[AsIntegral(EConstantBufferSlot::PerDirectionalLight)]]->SetPrivateData(WKPDID_D3DDebugObjectName, 19, "PerDirectionalLight");
		g_bufferStore[m_constantBuffers[AsIntegral(EConstantBufferSlot::PerMaterial)]]->SetPrivateData(WKPDID_D3DDebugObjectName, 11, "PerMaterial");
		g_bufferStore[m_constantBuffers[AsIntegral(EConstantBufferSlot::PerDraw)]]->SetPrivateData(WKPDID_D3DDebugObjectName, 7, "PerDraw");
#endif

		// Initialize our samplers
		//m_samplers.resize(AsIntegral(ESamplerSlot::MAX));
		m_samplers[AsIntegral(ESamplerSlot::Point)] = CreateSamplerState(D3D11_FILTER_MIN_MAG_MIP_POINT);
		m_samplers[AsIntegral(ESamplerSlot::Linear)] = CreateSamplerState(D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT);
		m_samplers[AsIntegral(ESamplerSlot::Trilinear)] = CreateSamplerState(D3D11_FILTER_MIN_MAG_MIP_LINEAR);
		m_samplers[AsIntegral(ESamplerSlot::Anisotropic)] = CreateSamplerState(D3D11_FILTER_ANISOTROPIC, 16);
		m_samplers[AsIntegral(ESamplerSlot::ShadowMap)] = CreateSamplerState(D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, 0, D3D11_TEXTURE_ADDRESS_BORDER, Color(1, 1, 1, 1));
		for (unsigned i = 0; i < m_samplers.size(); ++i)
		{
			SetPixelSamplerState(m_samplers[i], i);
		}

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

		g_dxgiSwapChain.Reset();
		g_d3dDeviceContext.Reset();
		g_d3dDevice.Reset();
	}

	void UGraphicsDriver::OnScreenSizeChanged()
	{
		if (!g_d3dDeviceContext) return;

		g_d3dDeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
		
		MAD_ASSERT_DESC(m_backBuffer != nullptr, "Back buffer should be valid if the device has been created");

		m_backBuffer.Reset();

		HR_CHECK(g_dxgiSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0), "Failed to resize swap chain");

		CreateBackBufferRenderTargetView();
	}

	SShaderResourceId UGraphicsDriver::CreateTextureFromFile(const eastl::string& inPath, uint64_t& outWidth, uint64_t& outHeight, bool inForceSRGB, bool inGenerateMips) const
	{
		auto widePath = utf8util::UTF16FromUTF8(inPath);
		auto extension = inPath.substr(inPath.find_last_of('.'));
		extension.make_lower();

		ComPtr<ID3D11ShaderResourceView> srv;
		ComPtr<ID3D11Resource> texture;

		HRESULT hr;
		if (extension == ".dds")
		{
			if (inGenerateMips)
			{
				hr = DirectX::CreateDDSTextureFromFileEx(g_d3dDevice.Get(), g_d3dDeviceContext.Get(), widePath.c_str(), 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0, inForceSRGB, texture.GetAddressOf(), srv.GetAddressOf());
			}
			else
			{
				hr = DirectX::CreateDDSTextureFromFileEx(g_d3dDevice.Get(), widePath.c_str(), 0, D3D11_USAGE_IMMUTABLE, D3D11_BIND_SHADER_RESOURCE, 0, 0, inForceSRGB, texture.GetAddressOf(), srv.GetAddressOf());
			}
		}
		else if (extension == ".png" || extension == ".bmp" || extension == ".jpeg" || extension == ".jpg" || extension == ".tif" || extension == ".tiff")
		{
			DirectX::WIC_LOADER_FLAGS flags = inForceSRGB ? DirectX::WIC_LOADER_FORCE_SRGB : DirectX::WIC_LOADER_DEFAULT;
			if (inGenerateMips)
			{
				hr = DirectX::CreateWICTextureFromFileEx(g_d3dDevice.Get(), g_d3dDeviceContext.Get(), widePath.c_str(), 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0, flags, texture.GetAddressOf(), srv.GetAddressOf());
			}
			else
			{
				hr = DirectX::CreateWICTextureFromFileEx(g_d3dDevice.Get(), widePath.c_str(), 0, D3D11_USAGE_IMMUTABLE, D3D11_BIND_SHADER_RESOURCE, 0, 0, flags, texture.GetAddressOf(), srv.GetAddressOf());
			}
		}
		else
		{
			LOG(LogTextureImport, Error, "Can only load textures of type DDS, PNG, JPG (JPEG), or BMP\n");
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

	bool UGraphicsDriver::CompileShaderFromFile(const eastl::string& inFileName, const eastl::string& inShaderEntryPoint, const eastl::string& inShaderModel, eastl::vector<char>& inOutCompileByteCode, const D3D_SHADER_MACRO* inShaderMacroDefines)
	{
		DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
		// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
		// Setting this flag improves the shader debugging experience, but still allows 
		// the shaders to be optimized and to run exactly the way they will run in 
		// the release configuration of this program.
		dwShaderFlags |= D3DCOMPILE_DEBUG;

		// Disable optimizations to further improve shader debugging
		dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		// Enable highest level of optimizations
		dwShaderFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

		eastl::wstring wideName = utf8util::UTF16FromUTF8(inFileName);

		ID3DBlob* pErrorBlob = nullptr;
		ID3DBlob* pBlobOut = nullptr;
		HRESULT hr = D3DCompileFromFile(wideName.c_str(), inShaderMacroDefines, D3D_COMPILE_STANDARD_FILE_INCLUDE, inShaderEntryPoint.c_str(), inShaderModel.c_str(),
			dwShaderFlags, 0, &pBlobOut, &pErrorBlob);

		if (FAILED(hr))
		{
			if (pErrorBlob)
			{
				LOG(LogGraphicsDevice, Error, "Failed to compile shader:\n\n%s\n", reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
				pErrorBlob->Release();
			}
			else
			{
				LOG(LogGraphicsDevice, Error, "Failed to compile shader:\n\nUNKNOWN ERROR\n");
			}

			return false;
		}

		if (pErrorBlob)
		{
			pErrorBlob->Release();
		}

		if (pBlobOut)
		{
			// Before releasing the blob, reflect the input layout from the shader (if vertex shader)
			if (inShaderModel.substr(0, 2) == "vs")
			{
				RegisterInputLayout(pBlobOut);
			}

			size_t compiledCodeSize = pBlobOut->GetBufferSize();
			inOutCompileByteCode.resize(compiledCodeSize);
			std::memcpy(inOutCompileByteCode.data(), pBlobOut->GetBufferPointer(), compiledCodeSize);

			pBlobOut->Release();
		}

		return hr == S_OK;
	}

	VertexShaderPtr_t UGraphicsDriver::CreateVertexShader(const eastl::vector<char>& inCompiledVSByteCode)
	{
		VertexShaderPtr_t vertexShaderPtr;

		DX_HRESULT(g_d3dDevice->CreateVertexShader(inCompiledVSByteCode.data(), inCompiledVSByteCode.size(), nullptr, vertexShaderPtr.GetAddressOf()), "Failure Creating Vertex Shader From Compiled Shader Code");

		return vertexShaderPtr;
	}

	PixelShaderPtr_t UGraphicsDriver::CreatePixelShader(const eastl::vector<char>& inCompiledPSByteCode)
	{
		PixelShaderPtr_t pixelShaderPtr;

		DX_HRESULT(g_d3dDevice->CreatePixelShader(inCompiledPSByteCode.data(), inCompiledPSByteCode.size(), nullptr, pixelShaderPtr.GetAddressOf()), "Failure Creating Pixel Shader from Compiled Shader Code");

		return pixelShaderPtr;
	}

	RenderTargetPtr_t UGraphicsDriver::CreateRenderTarget(UINT inWidth, UINT inHeight, DXGI_FORMAT inFormat, SShaderResourceId* outOptionalShaderResource) const
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
		HR_CHECK(g_d3dDevice->CreateTexture2D(&backingTexDesc, nullptr, backingTex.GetAddressOf()), "Failed to create backing texture for Render Target");

		D3D11_RENDER_TARGET_VIEW_DESC renderTargetDesc;
		MEM_ZERO(renderTargetDesc);
		renderTargetDesc.Format = backingTexDesc.Format;
		renderTargetDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		renderTargetDesc.Texture2D.MipSlice = 0;

		RenderTargetPtr_t renderTarget = nullptr;
		HR_CHECK(g_d3dDevice->CreateRenderTargetView(backingTex.Get(), &renderTargetDesc, renderTarget.GetAddressOf()), "Failed to create render target from backing texture");

		if (outOptionalShaderResource)
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceDesc;
			MEM_ZERO(shaderResourceDesc);
			shaderResourceDesc.Format = backingTexDesc.Format;
			shaderResourceDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			shaderResourceDesc.Texture2D.MostDetailedMip = 0;
			shaderResourceDesc.Texture2D.MipLevels = 1;

			ComPtr<ID3D11ShaderResourceView> shaderResource;
			HR_CHECK(g_d3dDevice->CreateShaderResourceView(backingTex.Get(), &shaderResourceDesc, shaderResource.GetAddressOf()), "Failed to create shader resource view from backing texture");

			auto shaderResourceId = SShaderResourceId::Next();
			g_shaderResourceViewStore.insert({ shaderResourceId, shaderResource });
			*outOptionalShaderResource = shaderResourceId;
		}

		return renderTarget;
	}

	InputLayoutPtr_t UGraphicsDriver::CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* inElements, UINT inNumElements, const eastl::vector<char>& inCompiledVertexShader) const
	{
		return CreateInputLayout(inElements, inNumElements, inCompiledVertexShader.data(), inCompiledVertexShader.size());
	}

	InputLayoutPtr_t UGraphicsDriver::CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* inElements, UINT inNumElements, const void* inCompiledVSByteCode, size_t inByteCodeSize) const
	{
		InputLayoutPtr_t inputLayoutPtr;

		HR_CHECK(g_d3dDevice->CreateInputLayout(inElements, inNumElements, inCompiledVSByteCode, inByteCodeSize, inputLayoutPtr.GetAddressOf()), "Failed creating input layout");

		return inputLayoutPtr;
	}

	SamplerStatePtr_t UGraphicsDriver::CreateSamplerState(D3D11_FILTER inFilterMode, UINT inMaxAnisotropy, D3D11_TEXTURE_ADDRESS_MODE inAddressMode, Color inBorderColor) const
	{
		D3D11_SAMPLER_DESC samplerDesc;
		MEM_ZERO(samplerDesc);
		samplerDesc.Filter = inFilterMode;
		samplerDesc.AddressU = inAddressMode;
		samplerDesc.AddressV = inAddressMode;
		samplerDesc.AddressW = inAddressMode;
		samplerDesc.MipLODBias = 0;
		samplerDesc.MaxAnisotropy = inMaxAnisotropy;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		if (D3D11_DECODE_IS_COMPARISON_FILTER(inFilterMode))
		{
			samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
		}

		if (inAddressMode == D3D11_TEXTURE_ADDRESS_BORDER)
		{
			samplerDesc.BorderColor[0] = inBorderColor.x;
			samplerDesc.BorderColor[1] = inBorderColor.y;
			samplerDesc.BorderColor[2] = inBorderColor.z;
			samplerDesc.BorderColor[3] = inBorderColor.w;
		}
		
		SamplerStatePtr_t samplerStatePtr = nullptr;
		HR_CHECK(g_d3dDevice->CreateSamplerState(&samplerDesc, samplerStatePtr.GetAddressOf()), "Failed to create sampler state");

		return samplerStatePtr;
	}

	DepthStencilPtr_t UGraphicsDriver::CreateDepthStencil(int inWidth, int inHeight, SShaderResourceId* outOptionalShaderResource) const
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
		HR_CHECK(g_d3dDevice->CreateTexture2D(&backingTexDesc, nullptr, backingTex.GetAddressOf()), "Failed to create backing texture for depth stencil");

		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilDesc;
		MEM_ZERO(depthStencilDesc);
		depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilDesc.Texture2D.MipSlice = 0;

		DepthStencilPtr_t depthStencil = nullptr;
		HR_CHECK(g_d3dDevice->CreateDepthStencilView(backingTex.Get(), &depthStencilDesc, depthStencil.GetAddressOf()), "Failed to create depth stencil view from backing texture");

		if (outOptionalShaderResource)
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceDesc;
			MEM_ZERO(shaderResourceDesc);
			shaderResourceDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			shaderResourceDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			shaderResourceDesc.Texture2D.MostDetailedMip = 0;
			shaderResourceDesc.Texture2D.MipLevels = 1;

			ComPtr<ID3D11ShaderResourceView> shaderResource;
			HR_CHECK(g_d3dDevice->CreateShaderResourceView(backingTex.Get(), &shaderResourceDesc, shaderResource.GetAddressOf()), "Failed to create shader resource view from backing texture");

			auto shaderResourceId = SShaderResourceId::Next();
			g_shaderResourceViewStore.insert({ shaderResourceId, shaderResource });
			*outOptionalShaderResource = shaderResourceId;
		}

		return depthStencil;
	}

	DepthStencilStatePtr_t UGraphicsDriver::CreateDepthStencilState(bool inDepthTestEnable, D3D11_COMPARISON_FUNC inComparisonFunc) const
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

		DepthStencilStatePtr_t depthStencilStatePtr = nullptr;
		HR_CHECK(g_d3dDevice->CreateDepthStencilState(&stateDesc, depthStencilStatePtr.GetAddressOf()), "Failed to create depth stencil state");

		return depthStencilStatePtr;
	}

	RasterizerStatePtr_t UGraphicsDriver::CreateRasterizerState(D3D11_FILL_MODE inFillMode, D3D11_CULL_MODE inCullMode) const
	{
		D3D11_RASTERIZER_DESC1 rasterDesc;
		MEM_ZERO(rasterDesc);
		rasterDesc.FillMode = inFillMode;
		rasterDesc.CullMode = inCullMode;
		rasterDesc.FrontCounterClockwise = true;
		rasterDesc.DepthClipEnable = true;

		RasterizerStatePtr_t rasterizerStatePtr = nullptr;
		HR_CHECK(g_d3dDevice->CreateRasterizerState1(&rasterDesc, rasterizerStatePtr.GetAddressOf()), "Failed to create rasterizer state");

		return rasterizerStatePtr;
	}

	RasterizerStatePtr_t UGraphicsDriver::CreateDepthRasterizerState() const
	{
		D3D11_RASTERIZER_DESC1 rasterDesc;
		MEM_ZERO(rasterDesc);
		rasterDesc.FillMode = D3D11_FILL_SOLID;
		rasterDesc.CullMode = D3D11_CULL_BACK;
		rasterDesc.FrontCounterClockwise = true;
		rasterDesc.DepthClipEnable = true;
		rasterDesc.DepthBias = 10000;
		rasterDesc.DepthBiasClamp = 0.0f;
		rasterDesc.SlopeScaledDepthBias = 1.5f;

		RasterizerStatePtr_t depthRasterizerStatePtr = nullptr;
		HR_CHECK(g_d3dDevice->CreateRasterizerState1(&rasterDesc, depthRasterizerStatePtr.GetAddressOf()), "Failed to create rasterizer state");

		return depthRasterizerStatePtr;
	}

	BlendStatePtr_t UGraphicsDriver::CreateBlendState(bool inEnableBlend) const
	{
		D3D11_BLEND_DESC1 blendDesc;
		MEM_ZERO(blendDesc);
		blendDesc.AlphaToCoverageEnable = FALSE;
		blendDesc.IndependentBlendEnable = FALSE;
		blendDesc.RenderTarget[0].BlendEnable = inEnableBlend;
		blendDesc.RenderTarget[0].LogicOpEnable = FALSE;
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].LogicOp = D3D11_LOGIC_OP_NOOP;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		BlendStatePtr_t blendStatePtr = nullptr;
		HR_CHECK(g_d3dDevice->CreateBlendState1(&blendDesc, blendStatePtr.GetAddressOf()), "Failed to create blend state");

		return blendStatePtr;
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
		HR_CHECK(g_d3dDevice->CreateBuffer(&bufferDesc, inData ? &initialData : nullptr, buffer.GetAddressOf()), "Failed to create graphics buffer");

		auto bufferId = SBufferId::Next();
		g_bufferStore.insert({ bufferId, buffer });
		return bufferId;
	}

	SBufferId UGraphicsDriver::CreateVertexBuffer(const void* inData, UINT inDataSize, D3D11_USAGE inUsageFlags, UINT inCPUAccessFlags) const
	{
		MAD_ASSERT_DESC(inData != nullptr, "Must specify initial vertex data");
		return CreateBuffer(inData, inDataSize, inUsageFlags, D3D11_BIND_VERTEX_BUFFER, inCPUAccessFlags);
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

	void UGraphicsDriver::SetRenderTargets(const RenderTargetPtr_t* inRenderTargets, int inNumRenderTargets, const DepthStencilPtr_t inOptionalDepthStencil) const
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
			/*const auto& renderTargetId = inRenderTargets[i];


			ID_GET_SAFE(renderTarget, renderTargetId, g_renderTargetStore, "Invalid render target");
			renderTargets[i] = renderTarget.Get();*/
			renderTargets[i] = inRenderTargets[i].Get();
		}

		ID3D11DepthStencilView* depthStencil = nullptr;
		if (inOptionalDepthStencil)
		{
			depthStencil = inOptionalDepthStencil.Get();
		}

		g_d3dDeviceContext->OMSetRenderTargets(inNumRenderTargets, renderTargets, depthStencil);
	}

	void UGraphicsDriver::SetDepthStencilState(DepthStencilStatePtr_t inDepthStencilState, UINT inStencilRef) const
	{
		// Doesn't require any null checks because setting a null depth stencil state will force the use of the default depth stencil state
		g_d3dDeviceContext->OMSetDepthStencilState(inDepthStencilState.Get(), inStencilRef);
	}

	void UGraphicsDriver::SetInputLayout(InputLayoutPtr_t inInputLayout) const
	{
		MAD_ASSERT_DESC(inInputLayout != nullptr, "Invalid input layout");
		g_d3dDeviceContext->IASetInputLayout(inInputLayout.Get());
	}

	void UGraphicsDriver::SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY inPrimitiveTopology) const
	{
		g_d3dDeviceContext->IASetPrimitiveTopology(inPrimitiveTopology);
	}

	void UGraphicsDriver::SetVertexBuffer(SBufferId inVertexBuffer, EVertexBufferSlot inVertexSlot, UINT inVertexSize, UINT inVertexOffset) const
	{
		ID_TRY_GET(buffer, inVertexBuffer, g_bufferStore, "Invalid vertex buffer");
		UINT byteOffset = inVertexOffset * inVertexSize;
		g_d3dDeviceContext->IASetVertexBuffers(AsIntegral(inVertexSlot), 1, buffer.GetAddressOf(), &inVertexSize, &byteOffset);
	}

	void UGraphicsDriver::SetIndexBuffer(SBufferId inIndexBuffer, UINT inIndexOffset) const
	{
		ID_TRY_GET(buffer, inIndexBuffer, g_bufferStore, "Invalid index buffer");
		g_d3dDeviceContext->IASetIndexBuffer(buffer.Get(), DXGI_FORMAT_R16_UINT, 2 * inIndexOffset);
	}

	void UGraphicsDriver::SetVertexShader(VertexShaderPtr_t inVertexShader) const
	{
		MAD_ASSERT_DESC(inVertexShader != nullptr, "Invalid vertex shader");

		g_d3dDeviceContext->VSSetShader(inVertexShader.Get(), nullptr, 0);
	}

	void UGraphicsDriver::SetVertexConstantBuffer(SBufferId inBuffer, UINT inSlot) const
	{
		ID_GET_SAFE(buffer, inBuffer, g_bufferStore, "Invalid constant buffer");
		g_d3dDeviceContext->VSSetConstantBuffers(inSlot, 1, buffer.GetAddressOf());
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

	void UGraphicsDriver::SetPixelShader(PixelShaderPtr_t inPixelShader) const
	{
		// No need for nullptr check because you can safely set the pixel shader to null
		g_d3dDeviceContext->PSSetShader(inPixelShader.Get(), nullptr, 0);
	}

	void UGraphicsDriver::SetPixelConstantBuffer(SBufferId inBuffer, UINT inSlot) const
	{
		ID_GET_SAFE(buffer, inBuffer, g_bufferStore, "Invalid constant buffer");
		g_d3dDeviceContext->PSSetConstantBuffers(inSlot, 1, buffer.GetAddressOf());
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

	void UGraphicsDriver::SetPixelSamplerState(SamplerStatePtr_t inSamplerState, UINT inSlot) const
	{
		// No need for nullptr check because setting a null sampler state will force the use of the default sampler state
		g_d3dDeviceContext->PSSetSamplers(inSlot, 1, inSamplerState.GetAddressOf());
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

	void UGraphicsDriver::SetRasterizerState(RasterizerStatePtr_t inRasterizerState) const
	{
		g_d3dDeviceContext->RSSetState(inRasterizerState.Get());
	}

	void UGraphicsDriver::SetBlendState(BlendStatePtr_t inBlendstate) const
	{
		// No need for nullptr check because setting a null blend state will force the use of the default blend state
		static FLOAT blendFactor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		g_d3dDeviceContext->OMSetBlendState(inBlendstate.Get(), blendFactor, 0xffffffff);
	}

	void UGraphicsDriver::DestroyDepthStencil(DepthStencilPtr_t& inOutDepthStencil) const
	{
		inOutDepthStencil.Reset();
		//ID_DESTROY(inOutDepthStencil, g_depthStencilStore);
	}

	void UGraphicsDriver::DestroyShaderResource(SShaderResourceId& inOutShaderResource) const
	{
		ID_DESTROY(inOutShaderResource, g_shaderResourceViewStore);
	}

	void UGraphicsDriver::DestroyRenderTarget(RenderTargetPtr_t& inOutRenderTarget) const
	{
		inOutRenderTarget.Reset();
		//ID_DESTROY(inOutRenderTarget, g_renderTargetStore);
	}

	void UGraphicsDriver::SetFullScreen(bool inIsFullscreen) const
	{
		HR_CHECK(g_dxgiSwapChain->SetFullscreenState(inIsFullscreen, nullptr), "Failed to set fullscreen state");
	}

	void UGraphicsDriver::ClearBackBuffer(const float inColor[4])
	{
		ClearRenderTarget(m_backBuffer, inColor);
	}

	void UGraphicsDriver::ClearRenderTarget(RenderTargetPtr_t inRenderTarget, const float inColor[4]) const
	{
		if (!inRenderTarget)
		{
			return;
		}

		g_d3dDeviceContext->ClearRenderTargetView(inRenderTarget.Get(), inColor);
	}

	void UGraphicsDriver::ClearDepthStencil(DepthStencilPtr_t inDepthStencil, bool inClearDepth, float inDepth, bool inClearStencil, UINT8 inStencil) const
	{
		if (!inDepthStencil)
		{
			return;
		}
		
		UINT clearFlags = 0;
		if (inClearDepth) clearFlags |= D3D11_CLEAR_DEPTH;
		if (inClearStencil) clearFlags |= D3D11_CLEAR_STENCIL;

		g_d3dDeviceContext->ClearDepthStencilView(inDepthStencil.Get(), clearFlags, inDepth, inStencil);
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

#ifdef _DEBUG
	void UGraphicsDriver::SetDebugName_RenderTarget(RenderTargetPtr_t inRenderTarget, const eastl::string& inName) const
	{
		MAD_ASSERT_DESC(inRenderTarget != nullptr, "Invalid render target");
		inRenderTarget->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(inName.size()), inName.data());
	}
#endif

	void UGraphicsDriver::DrawSubscreenQuad(const Vector4& inNDCQuadMin, const Vector4& inNDCQuadMax)
	{
		enum ESubscreenVert
		{
			TOP_LEFT_VERT,
			TOP_RIGHT_VERT,
			BOTTOM_RIGHT_VERT,
			BOTTOM_LEFT_VERT,
			VERT_CORNER_MAX
		};

		static eastl::vector<Vector3> SubscreenQuadVerts = { { -1.0f, 1.0f, 0.0f },{ 1.0f, 1.0f, 0.0f },{ 1.0f, -1.0f, 0.0f },{ -1.0f, -1.0f, 0.0f } };
		static const eastl::vector<uint16_t> SubscreenQuadIndices = { 0, 3, 1, 1, 3, 2 };
		static SBufferId SubscreenVertexBuffer = CreateVertexBuffer(SubscreenQuadVerts.data(), static_cast<UINT>(SubscreenQuadVerts.size() * sizeof(Vector3)), D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
		static SBufferId SubscreenIndexBuffer = CreateIndexBuffer(SubscreenQuadIndices.data(), static_cast<UINT>(SubscreenQuadIndices.size() * sizeof(uint16_t)));

		static InputLayoutPtr_t PosInputLayout = UInputLayoutCache::GetInputLayout(UInputLayoutCache::GetFlagForSemanticName("POSITION"));
		static RasterizerStatePtr_t SubscreenRasterState = CreateRasterizerState(D3D11_FILL_SOLID, D3D11_CULL_BACK);
		static DepthStencilStatePtr_t SubscreenDepthState = CreateDepthStencilState(false, D3D11_COMPARISON_ALWAYS);

		Vector3 subscreenVerts[VERT_CORNER_MAX];

		subscreenVerts[TOP_RIGHT_VERT] = Vector3(inNDCQuadMax.x, inNDCQuadMax.y, 0.0f);
		subscreenVerts[BOTTOM_LEFT_VERT] = Vector3(inNDCQuadMin.x, inNDCQuadMin.y, 0.0f);
		subscreenVerts[TOP_LEFT_VERT] = Vector3(subscreenVerts[BOTTOM_LEFT_VERT].x, subscreenVerts[BOTTOM_LEFT_VERT].y + (subscreenVerts[TOP_RIGHT_VERT].y - subscreenVerts[BOTTOM_LEFT_VERT].y), 0.0f);
		subscreenVerts[BOTTOM_RIGHT_VERT] = Vector3(subscreenVerts[TOP_RIGHT_VERT].x, subscreenVerts[TOP_RIGHT_VERT].y - (subscreenVerts[TOP_RIGHT_VERT].y - subscreenVerts[BOTTOM_LEFT_VERT].y), 0.0f);

		UpdateBuffer(SubscreenVertexBuffer, subscreenVerts, sizeof(subscreenVerts));

		SetDepthStencilState(SubscreenDepthState, 0);
		SetRasterizerState(SubscreenRasterState);
		SetInputLayout(PosInputLayout);
		SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		SetIndexBuffer(SubscreenIndexBuffer, 0);
		SetVertexBuffer(SubscreenVertexBuffer, EVertexBufferSlot::Position, sizeof(Vector3), 0);

		DrawIndexed(static_cast<int>(SubscreenQuadIndices.size()), 0, 0);
	}

	void UGraphicsDriver::DrawFullscreenQuad()
	{
		const Vector4 fullscreenQuadMin(-1.0f, -1.0f, 0.0, 1.0f);
		const Vector4 fullscreenQuadMax(1.0f, 1.0f, 0.0f, 1.0f);

		DrawSubscreenQuad(fullscreenQuadMin, fullscreenQuadMax);
	}

	ComPtr<ID3D11Device2> UGraphicsDriver::TEMPGetDevice()
	{
		return g_d3dDevice;
	}

	ComPtr<ID3D11DeviceContext2> UGraphicsDriver::TEMPGetDeviceContext()
	{
		return g_d3dDeviceContext;
	}

	void UGraphicsDriver::StartEventGroup(const eastl::wstring& inName)
	{
#ifdef _DEBUG
		if (g_d3dEvent)
		{
			g_d3dEvent->BeginEvent(inName.c_str());
		}
#else
		(void)inName;
#endif
	}

	void UGraphicsDriver::EndEventGroup()
	{
#ifdef _DEBUG
		if (g_d3dEvent)
		{
			g_d3dEvent->EndEvent();
		}
#endif
	}
}
