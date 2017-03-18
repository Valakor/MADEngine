#pragma once

#include "Rendering/VertexArray.h"
#include "Rendering/InputLayoutCache.h"

#include <EASTL/vector.h>

namespace MAD
{
	class UVertexBuffer
	{
	public:
		void AddVertexArray(EVertexBufferSlot inVBSlot, EInputLayoutSemantic::Type inLayoutSemantic, const void* inData, uint32_t inVertexSize, uint32_t inVertexCount);
		void BindToPipeline(uint32_t inVBOffset = 0);
	private:
		eastl::vector<UVertexArray> m_vertexArrays;
	};
}