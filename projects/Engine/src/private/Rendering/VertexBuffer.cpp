#include "Rendering/VertexBuffer.h"
#include "Rendering/RenderContext.h"

namespace MAD
{
	void UVertexBuffer::AddVertexArray(EVertexBufferSlot inVBSlot, EInputLayoutSemantic::Type inLayoutSemantic, const void* inData, uint32_t inVertexSize, uint32_t inVertexCount)
	{
		m_vertexArrays.emplace_back(URenderContext::Get().GetGraphicsDriver(), inVBSlot, inLayoutSemantic, inData, inVertexSize, inVertexCount);
	}

	void UVertexBuffer::BindToPipeline(uint32_t inVBOffset /*=0*/)
	{
		for (const auto& currentVA : m_vertexArrays)
		{
			currentVA.Bind(URenderContext::Get().GetGraphicsDriver(), inVBOffset);
		}
	}
}
