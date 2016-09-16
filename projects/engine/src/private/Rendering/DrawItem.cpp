#include "Rendering/DrawItem.h"
#include "Rendering/GraphicsDriver.h"
#include "Core/GameEngine.h"
#include "Rendering/Renderer.h"

namespace MAD
{
	SDrawItem::SDrawItem(): m_vertexSize(0)
	                      , m_vertexBufferOffset(0)
	                      , m_indexOffset(0)
	                      , m_indexCount(0) { }

	void SDrawItem::Draw(UGraphicsDriver& inGraphicsDriver, bool inBindMaterialProperties) const
	{
		inGraphicsDriver.SetVertexBuffer(m_vertexBuffer, m_vertexSize, m_vertexBufferOffset);
		inGraphicsDriver.SetIndexBuffer(m_indexBuffer, m_indexOffset);

		if (inBindMaterialProperties)
		{
			for (const auto& cBufferData : m_constantBufferData)
			{
				if (cBufferData.first == EConstantBufferSlot::PerDraw)
				{
					auto perDraw = *reinterpret_cast<const SPerDrawConstants*>(cBufferData.second.first);
					auto& perFrame = gEngine->GetRenderer().GetCameraConstants();

					perDraw.m_objectToViewMatrix = perDraw.m_objectToWorldMatrix * perFrame.m_cameraViewMatrix;
					perDraw.m_objectToProjectionMatrix = perDraw.m_objectToWorldMatrix * perFrame.m_cameraViewProjectionMatrix;
					inGraphicsDriver.UpdateBuffer(EConstantBufferSlot::PerDraw, &perDraw, cBufferData.second.second);
				}
				else
				{
					inGraphicsDriver.UpdateBuffer(cBufferData.first, cBufferData.second.first, cBufferData.second.second);
				}
			}

			for (const auto& textureData : m_textures)
			{
				inGraphicsDriver.SetPixelShaderResource(textureData.second, textureData.first);
			}
		}

		inGraphicsDriver.DrawIndexed(m_indexCount, 0, 0);
	}
}
