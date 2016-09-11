#include "Rendering/DrawItem.h"
#include "Rendering/GraphicsDriver.h"

namespace MAD
{
	void SDrawItem::Draw(UGraphicsDriver& inGraphicsDriver, bool inBindMaterialProperties) const
	{
		inGraphicsDriver.SetVertexBuffer(m_vertexBuffer, m_vertexSize, m_vertexBufferOffset);
		inGraphicsDriver.SetIndexBuffer(m_indexBuffer, m_indexOffset);

		if (inBindMaterialProperties)
		{
			for (const auto& cBufferData : m_constantBufferData)
			{
				inGraphicsDriver.UpdateBuffer(cBufferData.first, cBufferData.second.first, cBufferData.second.second);
			}

			for (const auto& textureData : m_textures)
			{
				inGraphicsDriver.SetPixelShaderResource(textureData.second, textureData.first);
			}
		}

		inGraphicsDriver.DrawIndexed(m_indexCount, 0, 0);
	}
}
