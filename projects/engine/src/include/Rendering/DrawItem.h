#pragma once

#include <cstdint>
#include <EASTL/vector.h>
#include <EASTL/utility.h>

#include "Rendering/GraphicsDriverTypes.h"
#include "Rendering/RenderingCommon.h"

namespace MAD
{
	struct SDrawItem
	{
		void Draw(class UGraphicsDriver& inGraphicsDriver) const;

		SBufferId m_vertexBuffer;
		size_t m_vertexBufferOffset;

		SBufferId m_indexBuffer;
		size_t m_indexOffset;
		size_t m_indexCount;

		eastl::vector<eastl::pair<EConstantBufferSlot, const void*>> m_constantBufferData;

		eastl::vector<eastl::pair<ETextureSlot, SShaderResourceId>> m_textures;
	};
}
