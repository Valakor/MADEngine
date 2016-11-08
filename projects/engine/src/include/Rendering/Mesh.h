#pragma once

#include <EASTL/shared_ptr.h>
#include <EASTL/string.h>
#include <EASTL/vector.h>

#include "Core/SimpleMath.h"
#include "Rendering/RenderingCommon.h"
#include "Rendering/GraphicsDriverTypes.h"
#include "Rendering/Material.h"
#include "Rendering/SubMesh.h"
#include "Rendering/VertexArray.h"

namespace MAD
{
	using Index_t = uint16_t;

	struct SMeshInstance
	{
		SMeshInstance() : m_bVisible(false) {}

		SPerDrawConstants m_perDrawConstants;
		eastl::shared_ptr<class UMesh> m_mesh;
		bool m_bVisible;
	};

	class UMesh
	{
	public:
		static SMeshInstance CreatePrimitivePlane();

		void BuildDrawItems(eastl::vector<struct SDrawItem>& inOutTargetDrawItems, const SPerDrawConstants& inPerMeshDrawConstants) const;

	//private:
		friend class UAssetCache;
		static eastl::shared_ptr<UMesh> Load(const eastl::string& inPath);

		eastl::vector<SSubMesh> m_subMeshes;
		eastl::vector<UMaterial> m_materials;

		SInputLayoutId m_inputLayout;

		eastl::vector<Vector3> m_positions;
		eastl::vector<Vector3> m_normals;
		eastl::vector<Vector3> m_tangents;
		eastl::vector<Vector2> m_texCoords;

		UVertexArray m_gpuPositions;
		UVertexArray m_gpuNormals;
		UVertexArray m_gpuTangents;
		UVertexArray m_gpuTexCoords;

		eastl::vector<Index_t> m_indexBuffer;
		SBufferId m_gpuIndexBuffer;
	};
}
