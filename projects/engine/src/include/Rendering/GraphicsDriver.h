#pragma once

#include <EASTL/string.h>
#include <EASTL/vector.h>

#include "Rendering/GraphicsDriverTypes.h"
#include "Rendering/RenderingCommon.h"

namespace MAD
{
	class UGraphicsDriver
	{
	public:
		UGraphicsDriver();

		bool Init(class UGameWindow& inWindow);
		void Shutdown();

		void OnScreenSizeChanged();

		SRenderTargetId GetBackBufferRenderTarget() const { return m_backBuffer; }

		// Don't use this directly, use the AssetCache interface, e.g.
		//     AssetCache.Load<UTexture>(...);
		SShaderResourceId CreateTextureFromFile(const eastl::string& inPath, uint64_t& outWidth, uint64_t& outHeight) const;
		
		bool CompileShaderFromFile(const eastl::string& inFileName, const eastl::string& inShaderEntryPoint, const eastl::string& inShaderModel, eastl::vector<char>& inOutCompileByteCode, SInputLayoutId* outOptInputLayout = nullptr);

		SVertexShaderId CreateVertexShader(const eastl::vector<char>& inCompiledVSByteCode);
		SPixelShaderId CreatePixelShader(const eastl::vector<char>& inCompiledPSByteCode);

		SRenderTargetId CreateRenderTarget(UINT inWidth, UINT inHeight, DXGI_FORMAT inFormat, SShaderResourceId* outOptionalShaderResource = nullptr) const;
		SInputLayoutId CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* inElements, UINT inNumElements, const eastl::vector<char>& inCompiledVertexShader) const;
		SDepthStencilId CreateDepthStencil(int inWidth, int inHeight, SShaderResourceId* outOptionalShaderResource = nullptr) const;
		SDepthStencilStateId CreateDepthStencilState(bool inDepthTestEnable, D3D11_COMPARISON_FUNC inComparisonFunc) const;
		SRasterizerStateId CreateRasterizerState(D3D11_FILL_MODE inFillMode, D3D11_CULL_MODE inCullMode) const;
		SBlendStateId CreateBlendState(bool inEnableBlend) const;

		SBufferId CreateVertexBuffer(const void* inData, UINT inDataSize) const;
		SBufferId CreateIndexBuffer(const void* inData, UINT inDataSize) const;

		void UpdateBuffer(EConstantBufferSlot inSlot, const void* inData, size_t inDataSize) const;

		void SetRenderTargets(const SRenderTargetId* inRenderTargets, int inNumRenderTargets, const SDepthStencilId* inOptionalDepthStencil) const;
		void SetDepthStencilState(SDepthStencilStateId inDepthStencilState, UINT inStencilRef) const;
		void SetViewport(float inX, float inY, float inWidth, float inHeight) const;
		void SetViewport(int inX, int inY, int inWidth, int inHeight) const;
		void SetInputLayout(SInputLayoutId inInputLayout) const;
		void SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY inPrimitiveTopology) const;
		void SetVertexBuffer(SBufferId inVertexBuffer, UINT inVertexSize, UINT inVertexIndexOffset) const;
		void SetIndexBuffer(SBufferId inIndexBuffer, UINT inIndexOffset) const;
		void SetVertexShader(SVertexShaderId inVertexShader) const;
		void SetPixelShader(SPixelShaderId inPixelShader) const;
		void SetPixelShaderResource(SShaderResourceId inShaderResource, ETextureSlot inSlot) const;
		void SetRasterizerState(SRasterizerStateId inRasterizerState) const;
		void SetBlendState(SBlendStateId inBlendstate) const;

		void DestroyDepthStencil(SDepthStencilId& inOutDepthStencil) const;
		void DestroyShaderResource(SShaderResourceId& inOutShaderResource) const;
		void DestroyRenderTarget(SRenderTargetId& inOutRenderTarget) const;

		void SetFullScreen(bool inIsFullscreen) const;

		void ClearBackBuffer(const float inColor[4]);
		void ClearRenderTarget(SRenderTargetId inRenderTarget, const float inColor[4]) const;
		void ClearDepthStencil(SDepthStencilId inDepthStencil, bool inClearDepth, float inDepth, bool inClearStencil = false, UINT8 inStencil = 0) const;
		void Draw(int inVertexCount, int inStartVertex) const;
		void DrawIndexed(int inIndexCount, int inStartIndex, int inBaseVertex) const;
		void Present() const;

	private:
		void CreateBackBufferRenderTargetView();
		SInputLayoutId CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* inElements, UINT inNumElements, const void* inCompiledVSByteCode, size_t inByteCodeSize) const;
		SInputLayoutId ReflectInputLayout(ID3DBlob* inTargetBlob) const;

		SSamplerStateId CreateSamplerState() const;
		void SetPixelSamplerState(SSamplerStateId inSamplerState, UINT inSlot) const;

		SBufferId CreateBuffer(const void* inData, UINT inDataSize, D3D11_USAGE inUsage, UINT inBindFlags, UINT inCpuAccessFlags) const;
		SBufferId CreateConstantBuffer(const void* inData, UINT inDataSize) const;

		void* MapBuffer(SBufferId inBuffer) const;
		void UnmapBuffer(SBufferId inBuffer) const;
		void UpdateBuffer(SBufferId inBuffer, const void* inData, size_t inDataSize) const;

		void SetVertexConstantBuffer(SBufferId inBuffer, UINT inSlot, UINT inOffset, UINT inLength) const;
		void SetPixelConstantBuffer(SBufferId inBuffer, UINT inSlot, UINT inOffset, UINT inLength) const;
		void SetPixelShaderResource(SShaderResourceId inShaderResource, UINT inSlot) const;

		SRenderTargetId m_backBuffer;

		eastl::vector<SBufferId> m_constantBuffers;
		eastl::vector<SSamplerStateId> m_samplers;
	};
}
