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

		OnScreenSizeChanged();

		InitializeTextVertexBuffers(inInitialNumChars);

		m_isBatchRendererEnabled = true;
	}

	void UTextBatchRenderer::OnScreenSizeChanged()
	{
		const POINT clientScreenSize = URenderContext::GetRenderer().GetScreenSize();

		m_textProjectionMatrix = Matrix::CreateOrthographic(clientScreenSize.x, clientScreenSize.y, -0.1f, 1.0f); // Negative near plane because text will have a 0.0f z value
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

		m_textBatchPosVB = graphicsDriver.CreateVertexBuffer(initialVertexPosData.data(), totalPosDataSize, EResourceUsage::Dynamic, ECPUAccess::Write);
		m_textBatchTexVB = graphicsDriver.CreateVertexBuffer(initialVertexUVData.data(), totalUVDataSize, EResourceUsage::Dynamic, ECPUAccess::Write);
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
		// Flushes all of the batched text instances
		if (!m_isBatchRendererEnabled)
		{
			return;
		}

		size_t totalVertexCount = 0;

		// Reserve enough vertices for the entire batch upfront
		eastl::for_each(m_queuedTextInstances.cbegin(), m_queuedTextInstances.cend(), [&totalVertexCount](const STextInstance& currentTextInst)
		{
			totalVertexCount += currentTextInst.SourceText.length() * UTextBatchRenderer::s_numVertsPerQuad;
		});

		m_runningVBPosData.reserve(totalVertexCount);

		for (auto& currentTextInstance : m_queuedTextInstances)
		{
			UpdateCPUTextData(currentTextInstance);
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

		auto& graphicsDriver = gEngine->GetRenderer().GetGraphicsDriver();
		memset(&updatedPerDrawContants, 0, sizeof(updatedPerDrawContants));

		graphicsDriver.StartEventGroup(L"Drawing Text Batch");

		graphicsDriver.SetInputLayout(UInputLayoutCache::GetInputLayout(EInputLayoutSemantic::Position | EInputLayoutSemantic::UV));
		graphicsDriver.SetPrimitiveTopology(EPrimitiveTopology::TriangleList);
		graphicsDriver.UpdateBuffer(m_textBatchPosVB, m_runningVBPosData.data(), m_runningVBPosData.size() * textVertPosSize);
		graphicsDriver.UpdateBuffer(m_textBatchTexVB, m_runningVBUVData.data(), m_runningVBUVData.size() * textVertUVSize);
		graphicsDriver.SetVertexBuffer(m_textBatchPosVB, EVertexBufferSlot::Position, textVertPosSize, 0);
		graphicsDriver.SetVertexBuffer(m_textBatchTexVB, EVertexBufferSlot::UV, textVertUVSize, 0);
		graphicsDriver.SetPixelShaderResource(m_textFontFamily->GetFontTextureResource(), ETextureSlot::DiffuseMap);

		// Set the appropriate constant buffer values (for world-view-projection matrix)
		updatedPerDrawContants.m_objectToProjectionMatrix = m_textProjectionMatrix;
		
		graphicsDriver.UpdateBuffer(EConstantBufferSlot::PerDraw, &updatedPerDrawContants, sizeof(updatedPerDrawContants));

		// Can't used index drawing because shared vertices between letters can't share UV data (since they are most likely different letters)
		graphicsDriver.Draw(static_cast<UINT>(m_runningVBPosData.size()), 0);

		// Unbind the font map as a shader resource (just in case, there is another use for the texture slot)
		graphicsDriver.SetPixelShaderResource(nullptr, ETextureSlot::DiffuseMap);

		graphicsDriver.EndEventGroup();
	}

	void UTextBatchRenderer::UpdateCPUTextData(const STextInstance& inTextInstance)
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