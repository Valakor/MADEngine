#include "Rendering/DrawItem.h"
#include "Rendering/GraphicsDriver.h"
#include "Rendering/Renderer.h"
#include "Core/GameEngine.h"
#include "Misc/ProgramPermutor.h"

namespace MAD
{
	SDrawItem::SDrawItem(): m_vertexBufferOffset(0)
	                      , m_indexOffset(0)
	                      , m_indexCount(0)
	                      , m_primitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED) { }

	void SDrawItem::Draw(UGraphicsDriver& inGraphicsDriver, bool inBindMaterialProperties) const
	{
		for (const auto& vertexBuffer : m_vertexBuffers)
		{
			vertexBuffer.Bind(inGraphicsDriver, m_vertexBufferOffset);
		}

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

			for (const auto& textureData : m_shaderResources)
			{
				inGraphicsDriver.SetPixelShaderResource(textureData.second, textureData.first);
			}
		}

		inGraphicsDriver.SetPrimitiveTopology(m_primitiveTopology);
		inGraphicsDriver.SetInputLayout(m_inputLayout);
		inGraphicsDriver.DrawIndexed(m_indexCount, 0, 0);
	}

	ProgramId_t SDrawItem::DetermineProgramId() const
	{
		ProgramId_t outputProgramId = 0;

		for (const auto& currentTextureSlot : m_shaderResources)
		{
			switch (currentTextureSlot.first)
			{
			case ETextureSlot::DiffuseMap: // Do you have a diffuse map?
				outputProgramId |= static_cast<ProgramId_t>(UProgramPermutor::EProgramIdMask::EProgramIdMask_Diffuse);
				break;
			case ETextureSlot::SpecularMap: // Do you have a specular map?
				outputProgramId |= static_cast<ProgramId_t>(UProgramPermutor::EProgramIdMask::EProgramIdMask_Specular);
				break;
			case ETextureSlot::EmissiveMap: // Do you have a emissive map?
				outputProgramId |= static_cast<ProgramId_t>(UProgramPermutor::EProgramIdMask::EProgramIdMask_Emissive);
				break;
			}
		}

		return outputProgramId;
	}

}
