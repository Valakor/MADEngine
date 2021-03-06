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

		eastl::shared_ptr<class UMesh> m_mesh;
		bool m_bVisible;
	};

	class UMesh
	{
	public:
		static SMeshInstance CreatePrimitivePlane();

		/*
		* Loads a mesh at the given path. The path should be relative to the assets root directory. Internally uses a cache
		* to ensure meshes are only loaded once. Returns null if the mesh cannot be loaded.
		*/
		static eastl::shared_ptr<UMesh> Load(const eastl::string& inRelativePath);

		void BuildDrawItems(eastl::vector<struct SDrawItem>& inOutTargetDrawItems, const ULinearTransform& inMeshTransform) const;

	//private:
		static eastl::shared_ptr<UMesh> Load_Internal(const eastl::string& inRelativePath);

		eastl::vector<SSubMesh> m_subMeshes;
		eastl::vector<UMaterial> m_materials;

		InputLayoutPtr_t m_inputLayout;

		eastl::vector<Vector3> m_positions;
		eastl::vector<Vector3> m_normals;
		eastl::vector<Vector4> m_tangents;
		eastl::vector<Vector2> m_texCoords;

		UVertexArray m_gpuPositions;
		UVertexArray m_gpuNormals;
		UVertexArray m_gpuTangents;
		UVertexArray m_gpuTexCoords;

		eastl::vector<Index_t> m_indexBuffer;
		BufferPtr_t m_gpuIndexBuffer;
	};
}
