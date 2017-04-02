#include "Rendering/DrawItem.h"
#include "Rendering/GraphicsDriver.h"
#include "Rendering/Renderer.h"
#include "Core/GameEngine.h"

namespace MAD
{
	SDrawItem::SDrawItem()
		: m_uniqueID(0)
		, m_previousDrawTransform(nullptr)
		, m_vertexBufferOffset(0)
		, m_vertexCount(0)
	    , m_indexOffset(0)
	    , m_indexCount(0)
	    , m_primitiveTopology(EPrimitiveTopology::Undefined)
	    , m_drawCommand(EDrawCommand::IndexedDraw) { } // TODO Rethink the default draw command later

	void SDrawItem::Draw(UGraphicsDriver& inGraphicsDriver, float inFramePercent, const SPerFrameConstants& inPerFrameConstants, bool inBindMaterialProperties, InputLayoutFlags_t inInputLayoutOverride, RasterizerStatePtr_t inRasterStateOverride)
	{
		InputLayoutFlags_t inputLayout = 0;
		for (const auto& vertexBuffer : m_vertexBuffers)
		{
			if (vertexBuffer.GetSemantic() & inInputLayoutOverride)
			{
				inputLayout |= vertexBuffer.GetSemantic();
				vertexBuffer.Bind(inGraphicsDriver, m_vertexBufferOffset);
			}
		}
		inGraphicsDriver.SetInputLayout(UInputLayoutCache::GetInputLayout(inputLayout));

		if (m_indexBuffer)
		{
			inGraphicsDriver.SetIndexBuffer(m_indexBuffer, m_indexOffset);
		}

		SPerDrawConstants perDrawConstants;

		if (m_previousDrawTransform)
		{
			// Do interpolation
			ULinearTransform interpedTransform = ULinearTransform::Lerp(*m_previousDrawTransform, m_transform, inFramePercent);
			perDrawConstants.m_objectToWorldMatrix = interpedTransform.GetMatrix();
		}
		else
		{
			perDrawConstants.m_objectToWorldMatrix = m_transform.GetMatrix();
		}

		perDrawConstants.m_objectToViewMatrix = perDrawConstants.m_objectToWorldMatrix * inPerFrameConstants.m_cameraViewMatrix;
		perDrawConstants.m_objectToProjectionMatrix = perDrawConstants.m_objectToWorldMatrix * inPerFrameConstants.m_cameraViewProjectionMatrix;
		inGraphicsDriver.UpdateBuffer(EConstantBufferSlot::PerDraw, &perDrawConstants, sizeof(perDrawConstants));

		if (inBindMaterialProperties)
		{
			for (const auto& cBufferData : m_constantBufferData)
			{
				MAD_ASSERT_DESC(cBufferData.first != EConstantBufferSlot::PerDraw, "PerDraw constants are always updated and shouldn't be here");
				inGraphicsDriver.UpdateBuffer(cBufferData.first, cBufferData.second.first, cBufferData.second.second);
			}

			for (const auto& textureData : m_shaderResources)
			{
				inGraphicsDriver.SetPixelShaderResource(textureData.second, textureData.first);
			}
		}

		if (inRasterStateOverride)
		{
			inGraphicsDriver.SetRasterizerState(inRasterStateOverride);
		}
		else
		{
			inGraphicsDriver.SetRasterizerState(m_rasterizerState);
		}

		inGraphicsDriver.SetPrimitiveTopology(m_primitiveTopology);

		switch (m_drawCommand)
		{
		case EDrawCommand::VertexDraw:
			inGraphicsDriver.Draw(m_vertexCount, 0);
			break;
		case EDrawCommand::IndexedDraw:
			inGraphicsDriver.DrawIndexed(m_indexCount, 0, 0);
			break;
		}
	}
}
