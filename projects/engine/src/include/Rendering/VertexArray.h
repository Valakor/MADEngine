#pragma once

#include "Rendering/GraphicsDriverTypes.h"
#include "Rendering/RenderingCommon.h"

namespace MAD
{
	class UVertexArray
	{
	public:
		UVertexArray();
		UVertexArray(class UGraphicsDriver& inGraphicsDriver, EVertexBufferSlot inSlot, const void* inVertexData, uint32_t inVertexSize, uint32_t inVertexCount);

		void Bind(class UGraphicsDriver& inGraphicsDriver, uint32_t inOffset) const;
		bool Empty() const { return m_bufferSize == 0; }

	private:
		SBufferId m_buffer;
		uint32_t m_vertexSize;
		uint32_t m_bufferSize;
		EVertexBufferSlot m_arrayUsage;
	};
}