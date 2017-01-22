#pragma once

#include <EASTL/string.h>
#include <EASTL/vector.h>

#include "Rendering/GraphicsDriverTypes.h"
#include "Rendering/RenderingCommon.h"

#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

namespace MAD
{
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
		SShaderResourceId CreateTextureFromFile(const eastl::string& inPath, uint64_t& outWidth, uint64_t& outHeight, bool inForceSRGB, bool inGenerateMips) const;
		
		bool CompileShaderFromFile(const eastl::string& inFileName, const eastl::string& inShaderEntryPoint, const eastl::string& inShaderModel, eastl::vector<char>& inOutCompileByteCode, const D3D_SHADER_MACRO* inShaderMacroDefines = nullptr);

		VertexShaderPtr_t CreateVertexShader(const eastl::vector<char>& inCompiledVSByteCode);
		PixelShaderPtr_t CreatePixelShader(const eastl::vector<char>& inCompiledPSByteCode);

		RenderTargetPtr_t CreateRenderTarget(UINT inWidth, UINT inHeight, DXGI_FORMAT inFormat, SShaderResourceId* outOptionalShaderResource = nullptr) const;
		InputLayoutPtr_t CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* inElements, UINT inNumElements, const eastl::vector<char>& inCompiledVertexShader) const;
		InputLayoutPtr_t CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* inElements, UINT inNumElements, const void* inCompiledVSByteCode, size_t inByteCodeSize) const;
		DepthStencilPtr_t CreateDepthStencil(int inWidth, int inHeight, SShaderResourceId* outOptionalShaderResource = nullptr) const;
		DepthStencilStatePtr_t CreateDepthStencilState(bool inDepthTestEnable, D3D11_COMPARISON_FUNC inComparisonFunc) const;
		RasterizerStatePtr_t CreateRasterizerState(D3D11_FILL_MODE inFillMode, D3D11_CULL_MODE inCullMode) const;
		RasterizerStatePtr_t CreateDepthRasterizerState() const;
		BlendStatePtr_t CreateBlendState(bool inEnableBlend) const;

		SBufferId CreateVertexBuffer(const void* inData, UINT inDataSize, D3D11_USAGE inUsageFlags = D3D11_USAGE_IMMUTABLE, UINT inCPUAccessFlags = 0) const;
		SBufferId CreateIndexBuffer(const void* inData, UINT inDataSize) const;

		void UpdateBuffer(EConstantBufferSlot inSlot, const void* inData, size_t inDataSize) const;

		void SetRenderTargets(const RenderTargetPtr_t* inRenderTargets, int inNumRenderTargets, const DepthStencilPtr_t inOptionalDepthStencil) const;
		void SetDepthStencilState(DepthStencilStatePtr_t inDepthStencilState, UINT inStencilRef) const;
		void SetViewport(float inX, float inY, float inWidth, float inHeight) const;
		void SetInputLayout(InputLayoutPtr_t inInputLayout) const;
		void SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY inPrimitiveTopology) const;
		void SetVertexBuffer(SBufferId inVertexBuffer, EVertexBufferSlot inVertexSlot, UINT inVertexSize, UINT inVertexIndexOffset) const;
		void SetIndexBuffer(SBufferId inIndexBuffer, UINT inIndexOffset) const;
		void SetVertexShader(VertexShaderPtr_t inVertexShader) const;
		void SetPixelShader(PixelShaderPtr_t inPixelShader) const;
		void SetPixelShaderResource(SShaderResourceId inShaderResource, ETextureSlot inSlot) const;
		void SetRasterizerState(RasterizerStatePtr_t inRasterizerState) const;
		void SetBlendState(BlendStatePtr_t inBlendstate) const;

		void DestroyDepthStencil(DepthStencilPtr_t& inOutDepthStencil) const;
		void DestroyShaderResource(SShaderResourceId& inOutShaderResource) const;
		void DestroyRenderTarget(RenderTargetPtr_t& inOutRenderTarget) const;

		void SetFullScreen(bool inIsFullscreen) const;

		void ClearBackBuffer(const float inColor[4]);
		void ClearRenderTarget(RenderTargetPtr_t inRenderTarget, const float inColor[4]) const;
		void ClearDepthStencil(DepthStencilPtr_t inDepthStencil, bool inClearDepth, float inDepth, bool inClearStencil = false, UINT8 inStencil = 0) const;
		void Draw(int inVertexCount, int inStartVertex) const;
		void DrawIndexed(int inIndexCount, int inStartIndex, int inBaseVertex) const;
		void Present() const;

#ifdef _DEBUG
		void SetDebugName_RenderTarget(RenderTargetPtr_t inRenderTarget, const eastl::string& inName) const;
#endif

		void StartEventGroup(const eastl::wstring& inName);
		void EndEventGroup();

		void DrawSubscreenQuad(const Vector4& inNDCQuadMin, const Vector4& inNDCQuadMax);
		void DrawFullscreenQuad();


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

		SBufferId CreateBuffer(const void* inData, UINT inDataSize, D3D11_USAGE inUsage, UINT inBindFlags, UINT inCpuAccessFlags) const;
		SBufferId CreateConstantBuffer(const void* inData, UINT inDataSize) const;

		void* MapBuffer(SBufferId inBuffer) const;
		void UnmapBuffer(SBufferId inBuffer) const;
		void UpdateBuffer(SBufferId inBuffer, const void* inData, size_t inDataSize) const;

		void SetVertexConstantBuffer(SBufferId inBuffer, UINT inSlot) const;
		void SetVertexConstantBuffer(SBufferId inBuffer, UINT inSlot, UINT inOffset, UINT inLength) const;
		void SetPixelConstantBuffer(SBufferId inBuffer, UINT inSlot) const;
		void SetPixelConstantBuffer(SBufferId inBuffer, UINT inSlot, UINT inOffset, UINT inLength) const;
		void SetPixelShaderResource(SShaderResourceId inShaderResource, UINT inSlot) const;

		RenderTargetPtr_t m_backBuffer;

		eastl::vector<SBufferId> m_constantBuffers;

		eastl::array<SamplerStatePtr_t, static_cast<size_t>(ESamplerSlot::MAX)> m_samplers;
		// Apparently there is an implicit conversion compiler error with using eastl::vector and ComPtrs (investigate!)
		//eastl::vector<SamplerStatePtr_t> m_samplers;
	};
}
