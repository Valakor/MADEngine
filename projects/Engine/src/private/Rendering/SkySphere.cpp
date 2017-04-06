#include "Rendering/SkySphere.h"
#include "Rendering/RenderContext.h"
#include "Rendering/Renderer.h"
#include "Rendering/GraphicsDriver.h"
#include "Rendering/DrawItem.h"
#include "Rendering/Texture.h"
#include "Rendering/RenderPassProgram.h"
#include "Rendering/Mesh.h"

#include "Misc/Assert.h"

namespace MAD
{
	USkySphere::USkySphere(const eastl::string& inShaderPath, const eastl::string& inCubemapPath, const Vector3& inBoxDimensions)
		: m_skyboxDimensions(inBoxDimensions)
		, m_skyboxTransform(Matrix::CreateScale(inBoxDimensions.x, inBoxDimensions.y, inBoxDimensions.z))
	{
		MAD_CHECK_DESC(InitializeSkybox(inShaderPath, inCubemapPath), "Error loading skybox\n");
	}

	void USkySphere::DrawSkybox()
	{
		auto& graphicsDriver = URenderContext::Get().GetGraphicsDriver();
		const auto& gBufferPassDesc = URenderContext::Get().GetRenderer().GetGBufferPassDescriptor();
		SPerDrawConstants skyboxDrawConstants;

		graphicsDriver.StartEventGroup(L"Drawing Sky Sphere");

		// Bind the program
		m_skyboxShader->SetProgramActive(graphicsDriver, 0);

		// Bind the input layout
		graphicsDriver.SetInputLayout(m_skyboxInputLayout);

		// Bind the vertex buffer
		m_skyboxMesh->m_gpuPositions.Bind(graphicsDriver, 0);
		
		graphicsDriver.SetIndexBuffer(m_skyboxMesh->m_gpuIndexBuffer, 0);

		// Set the light accumulation buffer as render target since we dont want that the skybox to be lit (remember to unbind the depth buffer as input incase previous steps needed it as a SRV)
		graphicsDriver.SetPixelShaderResource(nullptr, ETextureSlot::DepthBuffer);
		graphicsDriver.SetRenderTargets(&gBufferPassDesc.m_renderTargets[AsIntegral(ERenderTargetSlot::LightingBuffer)], 1, m_depthStencilView.Get());

		graphicsDriver.SetDepthStencilState(m_depthStencilState, 0);

		// Upload sky box's scale matrix
		skyboxDrawConstants.m_objectToWorldMatrix = m_skyboxTransform;
		graphicsDriver.UpdateBuffer(EConstantBufferSlot::PerDraw, &skyboxDrawConstants, sizeof(skyboxDrawConstants));

		// Bind the rasterizer state
		graphicsDriver.SetRasterizerState(m_boxRasterizerState);
		graphicsDriver.SetBlendState(m_skyboxBlendState);

		graphicsDriver.SetPixelShaderResource(m_boxCubeMapSRV, ETextureSlot::CubeMap);

		graphicsDriver.DrawIndexed(static_cast<UINT>(m_skyboxMesh->m_indexBuffer.size()), 0, 0);

		graphicsDriver.EndEventGroup();
	}

	bool USkySphere::InitializeSkybox(const eastl::string& inShaderPath, const eastl::string& inCubemapPath)
	{
		eastl::shared_ptr<UTexture> boxCubeMapTex = UTexture::Load(inCubemapPath, true, false, D3D11_RESOURCE_MISC_TEXTURECUBE);

		m_boxCubeMapSRV = boxCubeMapTex->GetTexureResource();
		MAD_CHECK_DESC(m_boxCubeMapSRV, "Error creating the shader resource view for the skybox cube map\n");
		if (!m_boxCubeMapSRV) return false;

		m_skyboxShader = URenderPassProgram::Load(inShaderPath);
		MAD_CHECK_DESC(m_skyboxShader != nullptr, "Error loading the skybox shader program\n");
		if (!m_skyboxShader) return false;

		m_skyboxInputLayout = UInputLayoutCache::GetInputLayout(EInputLayoutSemantic::Position);
		MAD_CHECK_DESC(m_skyboxInputLayout, "Error retrieving the skybox input layout\n");
		if (!m_skyboxInputLayout) return false;

		m_depthStencilView = URenderContext::Get().GetRenderer().GetGBufferPassDescriptor().m_depthStencilView;
		MAD_CHECK_DESC(m_depthStencilView, "Error retrieving the g-buffer's depth buffer\n");
		if (!m_depthStencilView) return false;

		m_depthStencilState = URenderContext::Get().GetRenderer().GetGBufferPassDescriptor().m_depthStencilState;

		m_boxRasterizerState = URenderContext::Get().GetGraphicsDriver().CreateRasterizerState(EFillMode::Solid, ECullMode::Front);
		MAD_CHECK_DESC(m_boxRasterizerState, "Error creating rasterizer state\n");
		if (!m_boxRasterizerState) return false;

		m_skyboxBlendState = URenderContext::Get().GetGraphicsDriver().CreateBlendState(false);
		MAD_CHECK_DESC(m_skyboxBlendState, "Error creating blend state\n");
		if (!m_skyboxBlendState) return false;

		// Initialize the position vertex buffer with the vertices of the entire box
		m_skyboxMesh = UMesh::Load("engine\\meshes\\primitives\\icosphere.obj");
		MAD_CHECK_DESC(m_skyboxMesh != nullptr, "Error loading the skybox cube mesh\n");
		if (!m_skyboxMesh) return false;

		return true;
	}
}