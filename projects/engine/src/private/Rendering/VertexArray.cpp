#include "Rendering/VertexArray.h"

#include "Rendering/GraphicsDriver.h"

namespace MAD
{
	UVertexArray::UVertexArray(): m_vertexSize(0)
	                            , m_bufferSize(0)
	                            , m_arrayUsage() {}

	UVertexArray::UVertexArray(class UGraphicsDriver& inGraphicsDriver, EVertexBufferSlot inSlot, const void* inVertexData, uint32_t inVertexSize, uint32_t inVertexCount)
	{
		m_vertexSize = inVertexSize;
		m_bufferSize = inVertexSize * inVertexCount;
		m_arrayUsage = inSlot;
		m_buffer = inGraphicsDriver.CreateVertexBuffer(inVertexData, m_bufferSize);
	}

	void UVertexArray::Bind(UGraphicsDriver& inGraphicsDriver, uint32_t inOffset) const
	{
		inGraphicsDriver.SetVertexBuffer(m_buffer, m_arrayUsage, m_vertexSize, inOffset);
	}
}
