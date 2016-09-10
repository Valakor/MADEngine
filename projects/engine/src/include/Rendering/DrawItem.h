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
		uint32_t m_vertexBufferOffset;

		SBufferId m_indexBuffer;
		uint32_t m_indexStart;
		uint32_t m_indexOffset;

		SInputLayoutId m_inputLayout;

		eastl::vector<eastl::pair<EConstantBufferSlot, void*>> m_constantBufferType;

		eastl::vector<eastl::pair<ETextureSlot, SShaderResourceId>> m_textures;
	};
}
