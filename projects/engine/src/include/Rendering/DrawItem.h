#pragma once

#include <EASTL/utility.h>
#include <EASTL/vector.h>

#include "Rendering/GraphicsDriverTypes.h"
#include "Rendering/RenderingCommon.h"

namespace MAD
{
	struct SDrawItem
	{
		SDrawItem();

		void Draw(class UGraphicsDriver& inGraphicsDriver, bool inBindMaterialProperties) const;

		SBufferId m_vertexBuffer;
		UINT m_vertexSize;
		UINT m_vertexBufferOffset;

		SBufferId m_indexBuffer;
		UINT m_indexOffset;
		UINT m_indexCount;

		eastl::vector<eastl::pair<EConstantBufferSlot, eastl::pair<const void*, UINT>>> m_constantBufferData;

		eastl::vector<eastl::pair<ETextureSlot, SShaderResourceId>> m_textures;
	};
}
