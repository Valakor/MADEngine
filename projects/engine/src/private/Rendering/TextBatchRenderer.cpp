#include "Rendering/TextBatchRenderer.h"
#include "Rendering/Renderer.h"
#include "Rendering/GraphicsDriver.h"
#include "Rendering/FontFamily.h"
#include "Rendering/RenderContext.h"
#include "Rendering/VertexDataTypes.h"
#include "Rendering/InputLayoutCache.h"

namespace MAD
{
	void UTextBatchRenderer::Init(const eastl::string& inFontRelativePath, uint16_t inInitialNumChars /*= 256*/)
	{
		m_textFontFamily = UFontFamily::Load(inFontRelativePath);

		const POINT clientScreenSize = URenderContext::GetRenderer().GetScreenSize();

		m_textProjectionMatrix = Matrix::CreateOrthographic(clientScreenSize.x, clientScreenSize.y, -0.1f, 1.0f); // Negative near plane because text will have a 0.0f z value

		InitializeTextVertexBuffers(inInitialNumChars);

		m_isBatchRendererEnabled = true;
	}

	void UTextBatchRenderer::InitializeTextVertexBuffers(uint16_t inInitialNumChars)
	{
		// Create a vertex buffer large enough to fit a minimum of 255 characters worth of data (expand if needed)
		eastl::vector<Vector3> initialVertexPosData;
		eastl::vector<Vector2> initialVertexUVData;
		auto graphicsDriver = URenderContext::Get().GetGraphicsDriver();
		UINT totalPosDataSize = 0;
		UINT totalUVDataSize = 0;

		initialVertexPosData.resize(inInitialNumChars * UTextBatchRenderer::s_numVertsPerQuad);
		initialVertexUVData.resize(inInitialNumChars * UTextBatchRenderer::s_numVertsPerQuad);

		totalPosDataSize = static_cast<UINT>(initialVertexPosData.size() * sizeof(Vector3));
		totalUVDataSize = static_cast<UINT>(initialVertexUVData.size() * sizeof(Vector2));

		memset(initialVertexPosData.data(), 0, totalPosDataSize);
		memset(initialVertexUVData.data(), 0, totalUVDataSize);

		m_textBatchPosVB = graphicsDriver.CreateVertexBuffer(initialVertexPosData.data(), totalPosDataSize, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
		m_textBatchTexVB = graphicsDriver.CreateVertexBuffer(initialVertexUVData.data(), totalUVDataSize, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
	}

	void UTextBatchRenderer::BatchTextInstance(const eastl::string& inSourceString, float inScreenX, float inScreenY)
	{
		if (!m_isBatchRendererEnabled)
		{
			return;
		}

		const POINT clientScreenSize = URenderContext::GetRenderer().GetScreenSize();
		auto& queuedInstance = m_queuedTextInstances.push_back();
	
		// Coordinates relative to the screen center
		float localCoordX = inScreenX - (clientScreenSize.x / 2.0f);
		float localCoordY = (clientScreenSize.y / 2.0f) - inScreenY;

		queuedInstance.SourceText = inSourceString;
		queuedInstance.LocalCoordX = localCoordX;
		queuedInstance.LocalCoordY = localCoordY;
	}

	void UTextBatchRenderer::FlushBatch()
	{
		if (!m_isBatchRendererEnabled)
		{
			return;
		}

		// Flushes all of the batched text instances
		size_t totalVertexCount = 0;

		// Reserve enough vertices for the entire batch upfront
		eastl::for_each(m_queuedTextInstances.cbegin(), m_queuedTextInstances.cend(), [&totalVertexCount](const STextInstance& currentTextInst)
		{
			totalVertexCount += currentTextInst.SourceText.length() * UTextBatchRenderer::s_numVertsPerQuad;
		});

		m_runningVBPosData.reserve(totalVertexCount);

		for (auto& currentTextInstance : m_queuedTextInstances)
		{
			ProcessTextInstance(currentTextInstance);
		}

		BindToPipeline();

		m_runningVBPosData.clear();
		m_runningVBUVData.clear();
		m_queuedTextInstances.clear();
	}

	void UTextBatchRenderer::BindToPipeline()
	{
		const UINT textVertPosSize = sizeof(Vector3);
		const UINT textVertUVSize = sizeof(Vector2);
		SPerDrawConstants updatedPerDrawContants;
		UINT byteOffset = 0;
		
		auto& graphicsDriver = gEngine->GetRenderer().GetGraphicsDriver();
		auto renderContext = graphicsDriver.TEMPGetDeviceContext();

		memset(&updatedPerDrawContants, 0, sizeof(updatedPerDrawContants));

		graphicsDriver.StartEventGroup(L"Drawing Text Batch");

		auto textInputLayout = UInputLayoutCache::GetInputLayout(EInputLayoutSemantic::Position | EInputLayoutSemantic::UV);

		renderContext->IASetInputLayout(textInputLayout.Get());

		// Update and bind the vertex buffers for the text quads
		graphicsDriver.UpdateBuffer(m_textBatchPosVB, m_runningVBPosData.data(), m_runningVBPosData.size() * textVertPosSize);
		graphicsDriver.UpdateBuffer(m_textBatchTexVB, m_runningVBUVData.data(), m_runningVBUVData.size() * textVertUVSize);

		renderContext->IASetVertexBuffers(AsIntegral(EVertexBufferSlot::Position), 1, m_textBatchPosVB.GetAddressOf(), &textVertPosSize, &byteOffset);
		renderContext->IASetVertexBuffers(AsIntegral(EVertexBufferSlot::UV), 1, m_textBatchTexVB.GetAddressOf(), &textVertUVSize, &byteOffset);

		renderContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		renderContext->PSSetShaderResources(AsIntegral(ETextureSlot::FontMap), 1, m_textFontFamily->GetFontTextureResource().GetAddressOf());

		// Set the appropriate constant buffer values (for world-view-projection matrix)
		updatedPerDrawContants.m_objectToProjectionMatrix = m_textProjectionMatrix;
		
		graphicsDriver.UpdateBuffer(EConstantBufferSlot::PerDraw, &updatedPerDrawContants, sizeof(updatedPerDrawContants));

		// TODO Why did I not use indexed rendering...? Probably should convert this to DrawIndexed
		renderContext->Draw(static_cast<UINT>(m_runningVBPosData.size()), 0);

		// Unbind the font map as a shader resource (just in case, there is another use for the texture slot)
		ShaderResourcePtr_t nullShaderResource = nullptr;
		renderContext->PSSetShaderResources(AsIntegral(ETextureSlot::FontMap), 1, nullShaderResource.GetAddressOf());

		graphicsDriver.EndEventGroup();
	}

	void UTextBatchRenderer::ProcessTextInstance(const STextInstance& inTextInstance)
	{
		// For each text instance, we need to produce the vertex data for the entire string
		float runningLocalX = inTextInstance.LocalCoordX;
		float runningLocalY = inTextInstance.LocalCoordY;
		eastl::vector<SFontChar> textStringFontInfo = m_textFontFamily->ConvertFromString(inTextInstance.SourceText);

		for (const auto& currentCharInfo : textStringFontInfo)
		{
			GenerateQuadVertices(currentCharInfo, runningLocalX, runningLocalY);

			runningLocalX += currentCharInfo.Spacing.SpacingGap + 2.5f;
		}
	}

	void UTextBatchRenderer::GenerateQuadVertices(const SFontChar& inCharInfo, float inLocalCoordX, float inLocalCoordY)
	{
		// Generate the s_numVertsPerQuad for the current char
		const float constantZOffset = 0.0f;
		float runningCoordX = inLocalCoordX;
		float runningCoordY = inLocalCoordY;
		VertexType_t leftTriangleVerts[s_numVertsPerQuad / 2];
		VertexType_t rightTriangleVerts[s_numVertsPerQuad / 2];

		const uint32_t textBitmapSize = m_textFontFamily->GetFontBitmapDimension();
		
		// TODO Probably could clean this up, but not really priority

		// Assume counter clockwise winding order of triangles
		// (Left Triangle)
		// 0 - 2
		// |  /
		// 1/
		//
		leftTriangleVerts[0].m_pos = Vector3({ runningCoordX, runningCoordY, constantZOffset });
		leftTriangleVerts[0].m_texCoord = Vector2({ inCharInfo.PosX / textBitmapSize, inCharInfo.PosY / textBitmapSize});

		leftTriangleVerts[1].m_pos = Vector3({runningCoordX, runningCoordY - inCharInfo.Height, constantZOffset });
		leftTriangleVerts[1].m_texCoord = Vector2({inCharInfo.PosX / textBitmapSize, (inCharInfo.PosY + inCharInfo.Height) / textBitmapSize});
		
		leftTriangleVerts[2].m_pos = Vector3({ runningCoordX + inCharInfo.Spacing.SpacingGap, runningCoordY, constantZOffset });
		leftTriangleVerts[2].m_texCoord = Vector2({(inCharInfo.PosX + inCharInfo.Spacing.SpacingGap) / textBitmapSize, inCharInfo.PosY / textBitmapSize});

		// (Right Triangle)
		//     0
		//	 / |
		//  /  |
		// 1 - 2
		//
		rightTriangleVerts[0] = leftTriangleVerts[2];
		rightTriangleVerts[1] = leftTriangleVerts[1];
		
		rightTriangleVerts[2].m_pos = Vector3({runningCoordX + inCharInfo.Spacing.SpacingGap, runningCoordY - inCharInfo.Height, constantZOffset });
		rightTriangleVerts[2].m_texCoord = Vector2({(inCharInfo.PosX + inCharInfo.Spacing.SpacingGap) / textBitmapSize, (inCharInfo.PosY + inCharInfo.Height) / textBitmapSize});

		for (size_t i = 0; i < (s_numVertsPerQuad / 2); ++i)
		{
			m_runningVBPosData.push_back(leftTriangleVerts[i].m_pos);
			m_runningVBUVData.push_back(leftTriangleVerts[i].m_texCoord);
		}

		for (size_t i = 0; i < (s_numVertsPerQuad / 2); ++i)
		{
			m_runningVBPosData.push_back(rightTriangleVerts[i].m_pos);
			m_runningVBUVData.push_back(rightTriangleVerts[i].m_texCoord);
		}
	}

}