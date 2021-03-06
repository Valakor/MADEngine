#pragma once

#include <EASTL/string.h>
#include <EASTL/vector.h>
#include <vector>

#include "Rendering/GraphicsDriverTypes.h"
#include "Rendering/RenderingCommon.h"

#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

namespace MAD
{
#ifdef _DEBUG
	// NOTE: name must be valid C++ variable name syntax (no spaces, etc.) since
	// Remotery uses it internally as a static variable a couple macros deep
#define GPU_EVENT_START(graphics, name)				\
	do												\
	{												\
		(graphics)->StartEventGroup(L#name);		\
		rmt_BeginD3D11Sample(name);					\
	} while (0)

#define GPU_EVENT_START_STR(graphics, name, str)	\
	do												\
	{												\
		(graphics)->StartEventGroup(str);			\
		rmt_BeginD3D11Sample(name);					\
	} while (0)

#define GPU_EVENT_END(graphics)						\
	do												\
	{												\
		(graphics)->EndEventGroup();				\
		rmt_EndD3D11Sample();						\
	} while (0)
#else
#define GPU_EVENT_START(...) (void)0
#define GPU_EVENT_START_STR(...) (void)0
#define GPU_EVENT_END(...) (void)0
#endif

	class UGraphicsDriver
	{
	public:
		UGraphicsDriver();

		bool Init(class UGameWindow& inWindow);
		void Shutdown();

		void OnScreenSizeChanged();

		RenderTargetPtr_t GetBackBufferRenderTarget() const { return m_backBuffer; }

		// Don't use this directly, use the AssetCache interface, e.g.
		//     AssetCache.Load<UTexture>(...);
		ShaderResourcePtr_t CreateTextureFromFile(const eastl::string& inPath, uint64_t& outWidth, uint64_t& outHeight, bool inForceSRGB, bool inGenerateMips, int32_t inMiscFlags = 0) const;
		
		bool CompileShaderFromFile(const eastl::string& inFileName, const eastl::string& inShaderEntryPoint, const eastl::string& inShaderModel, eastl::vector<char>& inOutCompileByteCode, const D3D_SHADER_MACRO* inShaderMacroDefines = nullptr);

		VertexShaderPtr_t CreateVertexShader(const eastl::vector<char>& inCompiledVSByteCode);
		PixelShaderPtr_t CreatePixelShader(const eastl::vector<char>& inCompiledPSByteCode);
		GeometryShaderPtr_t CreateGeometryShader(const eastl::vector<char>& inCompiledGSByteCode);

		RenderTargetPtr_t CreateRenderTarget(UINT inWidth, UINT inHeight, DXGI_FORMAT inFormat, ShaderResourcePtr_t* outOptionalShaderResource = nullptr) const;
		RenderTargetPtr_t CreateRenderTarget(ResourcePtr_t inBackingResource, const SRenderTargetViewDesc& inRenderTargetView) const;
		InputLayoutPtr_t CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* inElements, UINT inNumElements, const eastl::vector<char>& inCompiledVertexShader) const;
		InputLayoutPtr_t CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* inElements, UINT inNumElements, const void* inCompiledVSByteCode, size_t inByteCodeSize) const;
		DepthStencilPtr_t CreateDepthStencil(int inWidth, int inHeight, ShaderResourcePtr_t* outOptionalShaderResource = nullptr) const;
		DepthStencilPtr_t CreateDepthStencil(ResourcePtr_t inResource, const SDepthStencilViewDesc& inDepthStencilDesc) const;
		DepthStencilStatePtr_t CreateDepthStencilState(bool inDepthTestEnable, EComparisonFunc inComparisonFunc, EDepthWriteMask inDepthWriteMask = EDepthWriteMask::All) const;
		Texture2DPtr_t CreateTexture2D(const STexture2DDesc& inTextureDesc, const void* inInitialData = nullptr);
		ShaderResourcePtr_t CreateShaderResource(ResourcePtr_t inResource, DXGI_FORMAT inFormat, D3D11_SRV_DIMENSION inSRVDimension, uint32_t inMostDetailedMip, uint32_t inMipLevels) const;
		RasterizerStatePtr_t CreateRasterizerState(EFillMode inFillMode, ECullMode inCullMode) const;
		RasterizerStatePtr_t CreateDepthRasterizerState() const;
		BlendStatePtr_t CreateBlendState(bool inEnableBlend, EBlendFactor inSrcBlend = EBlendFactor::One, EBlendFactor inDestBlend = EBlendFactor::One, EBlendOp inBlendOp = EBlendOp::Add,
										 EBlendFactor inSrcAlphaBlend = EBlendFactor::One, EBlendFactor inDestAlphaBlend = EBlendFactor::Zero, EBlendOp inAlphaBlendOp = EBlendOp::Add) const;

		BufferPtr_t CreateVertexBuffer(const void* inData, UINT inDataSize, EResourceUsage inUsageFlags = EResourceUsage::Immutable, ECPUAccess inCPUAccessFlags = ECPUAccess::None) const;
		BufferPtr_t CreateIndexBuffer(const void* inData, UINT inDataSize) const;

		void UpdateBuffer(EConstantBufferSlot inSlot, const void* inData, size_t inDataSize) const;

		void SetRenderTargets(const RenderTargetPtr_t* inRenderTargets, int inNumRenderTargets, const DepthStencilPtr_t inOptionalDepthStencil) const;
		void SetDepthStencilState(DepthStencilStatePtr_t inDepthStencilState, UINT inStencilRef) const;
		void SetViewport(float inX, float inY, float inWidth, float inHeight) const;
		void SetViewport(const SGraphicsViewport& inViewPort) const;
		void SetInputLayout(InputLayoutPtr_t inInputLayout) const;
		void SetPrimitiveTopology(EPrimitiveTopology inPrimitiveTopology) const;
		void SetVertexBuffer(BufferPtr_t inVertexBuffer, VertexBufferSlotType_t inVertexSlot, UINT inVertexSize, UINT inVertexIndexOffset) const;
		void SetIndexBuffer(BufferPtr_t inIndexBuffer, UINT inIndexOffset) const;
		void SetVertexShader(VertexShaderPtr_t inVertexShader) const;
		void SetGeometryShader(GeometryShaderPtr_t inGeometryShader) const;
		void SetPixelShader(PixelShaderPtr_t inPixelShader) const;
		void SetPixelShaderResource(ShaderResourcePtr_t inShaderResource, ETextureSlot inSlot) const;
		void SetRasterizerState(RasterizerStatePtr_t inRasterizerState) const;
		void SetBlendState(BlendStatePtr_t inBlendstate) const;

		void DestroyDepthStencil(DepthStencilPtr_t& inOutDepthStencil) const;
		void DestroyShaderResource(ShaderResourcePtr_t& inOutShaderResource) const;
		void DestroyRenderTarget(RenderTargetPtr_t& inOutRenderTarget) const;

		void SetFullScreen(bool inIsFullscreen) const;

		void ClearBackBuffer(const Color& inColor);
		void ClearRenderTarget(RenderTargetPtr_t inRenderTarget, const Color& inColor = Color(0.0f, 0.0f, 0.0f, 0.0f)) const;
		void ClearDepthStencil(DepthStencilPtr_t inDepthStencil, bool inClearDepth, float inDepth, bool inClearStencil = false, UINT8 inStencil = 0) const;
		void Draw(int inVertexCount, int inStartVertex) const;
		void DrawIndexed(int inIndexCount, int inStartIndex, int inBaseVertex) const;
		void Present() const;

#ifdef _DEBUG
		void SetDebugName_RenderTarget(RenderTargetPtr_t inRenderTarget, const eastl::string& inName) const;

		void StartEventGroup(const eastl::wstring& inName);
		void EndEventGroup();
#endif

		void DrawSubscreenQuad(const Vector4& inNDCQuadMin, const Vector4& inNDCQuadMax);
		void DrawFullscreenQuad();

		void UpdateBuffer(BufferPtr_t inBuffer, const void* inData, size_t inDataSize) const;

		// EVENTUALLY RIP OUT AND REFACTOR ENTIRE GRAPHICS DRIVER
		//=================TESTING PURPOSES=========================
		ComPtr<ID3D11Device2> TEMPGetDevice();
		ComPtr<ID3D11DeviceContext2> TEMPGetDeviceContext();
		//==========================================================
	private:
		void CreateBackBufferRenderTargetView();
		void RegisterInputLayout(ID3DBlob* inTargetBlob);

		SamplerStatePtr_t CreateSamplerState(D3D11_FILTER inFilterMode, UINT inMaxAnisotropy = 0, D3D11_TEXTURE_ADDRESS_MODE inAddressMode = D3D11_TEXTURE_ADDRESS_WRAP, Color inBorderColor = Color()) const;
		void SetPixelSamplerState(SamplerStatePtr_t inSamplerState, UINT inSlot) const;

		BufferPtr_t CreateBuffer(const void* inData, UINT inDataSize, EResourceUsage inUsage, EBindFlag inBindFlags, ECPUAccess inCpuAccessFlags) const;
		BufferPtr_t CreateConstantBuffer(const void* inData, UINT inDataSize) const;

		void* MapBuffer(BufferPtr_t inBuffer) const;
		void UnmapBuffer(BufferPtr_t inBuffer) const;

		void SetVertexConstantBuffer(BufferPtr_t inBuffer, UINT inSlot) const;
		void SetVertexConstantBuffer(BufferPtr_t inBuffer, UINT inSlot, UINT inOffset, UINT inLength) const;
		void SetPixelConstantBuffer(BufferPtr_t inBuffer, UINT inSlot) const;
		void SetPixelConstantBuffer(BufferPtr_t inBuffer, UINT inSlot, UINT inOffset, UINT inLength) const;
		void SetGeometryConstantBuffer(BufferPtr_t inBuffer, UINT inSlot) const;
		void SetGeometryConstantBuffer(BufferPtr_t inBuffer, UINT inSlot, UINT inOffset, UINT inLength) const;
		void SetPixelShaderResource(ShaderResourcePtr_t inShaderResource, UINT inSlot) const;

		RenderTargetPtr_t m_backBuffer;

		eastl::vector<BufferPtr_t> m_constantBuffers;
		eastl::vector<SamplerStatePtr_t> m_samplers;
	};
}
