#pragma once

#include <EASTL/utility.h>
#include <EASTL/vector.h>

#include "Rendering/GraphicsDriverTypes.h"
#include "Rendering/RenderingCommon.h"
#include "Rendering/VertexArray.h"
#include "Rendering/RenderPassProgram.h"

namespace MAD
{
	struct SDrawItem
	{
		SDrawItem();

		void Draw(class UGraphicsDriver& inGraphicsDriver, bool inBindMaterialProperties) const;

		eastl::vector<UVertexArray> m_vertexBuffers;
		UINT m_vertexBufferOffset;

		SBufferId m_indexBuffer;
		UINT m_indexOffset;
		UINT m_indexCount;

		eastl::vector<eastl::pair<EConstantBufferSlot, eastl::pair<const void*, UINT>>> m_constantBufferData;
		eastl::vector<eastl::pair<ETextureSlot, SShaderResourceId>> m_shaderResources;

		SInputLayoutId m_inputLayout;

		D3D11_PRIMITIVE_TOPOLOGY m_primitiveTopology;

		SRasterizerStateId m_rasterizerState;
	};
}
