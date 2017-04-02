#pragma once

#include <EASTL/utility.h>
#include <EASTL/vector.h>

#include "Rendering/GraphicsDriverTypes.h"
#include "Rendering/RenderingCommon.h"
#include "Rendering/VertexArray.h"
#include "Rendering/InputLayoutCache.h"
#include "Rendering/Mesh.h"

namespace MAD
{
	enum class EDrawCommand : uint8_t
	{
		VertexDraw, // Vertex Buffer
		IndexedDraw, // Index Buffer

		MAX
	};

	struct SDrawItem
	{
		SDrawItem();

		void Draw(class UGraphicsDriver& inGraphicsDriver, float inFramePercent, const SPerFrameConstants& inPerFrameConstants, bool inBindMaterialProperties, InputLayoutFlags_t inInputLayoutOverride = eastl::numeric_limits<InputLayoutFlags_t>::max(), RasterizerStatePtr_t inRasterStateOverride = nullptr);

		// Used for renderer state ping-ponging for interpolating state
		size_t m_uniqueID;
		ULinearTransform* m_previousDrawTransform;

		eastl::vector<UVertexArray> m_vertexBuffers;
		UINT m_vertexCount;
		UINT m_vertexBufferOffset;

		BufferPtr_t m_indexBuffer;
		UINT m_indexOffset;
		UINT m_indexCount;

		ULinearTransform m_transform;

		eastl::vector<eastl::pair<EConstantBufferSlot, eastl::pair<const void*, UINT>>> m_constantBufferData;
		eastl::vector<eastl::pair<ETextureSlot, ShaderResourcePtr_t>> m_shaderResources;

		InputLayoutPtr_t m_inputLayout;

		EPrimitiveTopology m_primitiveTopology;

		RasterizerStatePtr_t m_rasterizerState;

		EDrawCommand m_drawCommand;
	};
}
