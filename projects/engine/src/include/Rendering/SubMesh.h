#pragma once

namespace MAD
{
	struct SSubMesh
	{
		size_t m_vertexStart;
		size_t m_vertexCount;

		size_t m_indexStart;
		size_t m_indexCount;

		size_t m_materialIndex;
	};
}
