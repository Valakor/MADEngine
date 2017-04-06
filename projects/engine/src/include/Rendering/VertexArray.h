#pragma once

#include "Rendering/GraphicsDriverTypes.h"
#include "Rendering/RenderingCommon.h"
#include "Rendering/InputLayoutCache.h"

namespace MAD
{
	class UVertexArray
	{
	public:
		UVertexArray();
		UVertexArray(class UGraphicsDriver& inGraphicsDriver, VertexBufferSlotType_t inSlot, EInputLayoutSemantic::Type inSemantic, 
					 const void* inVertexData, uint32_t inVertexSize, uint32_t inVertexCount,
					 EResourceUsage inUsage = EResourceUsage::Immutable, ECPUAccess inCPUAccessFlag = ECPUAccess::None);

		void Bind(class UGraphicsDriver& inGraphicsDriver, uint32_t inOffset) const;
		void Update(class UGraphicsDriver& inGraphicsDriver, const void* inData, size_t inDataSize);
		bool Empty() const { return m_bufferSize == 0; }
		EInputLayoutSemantic::Type GetSemantic() const { return m_arraySemantic; }

	private:
		BufferPtr_t m_buffer;
		uint32_t m_vertexSize;
		uint32_t m_bufferSize;
		VertexBufferSlotType_t m_arrayUsage;
		EInputLayoutSemantic::Type m_arraySemantic;
	};
}
