#pragma once

#include <EASTL/shared_ptr.h>
#include <EASTL/string.h>
#include <EASTL/vector.h>

#include "Core/SimpleMath.h"
#include "Rendering/RenderingCommon.h"
#include "Rendering/GraphicsDriverTypes.h"
#include "Rendering/Material.h"
#include "Rendering/SubMesh.h"

namespace MAD
{
	using Index_t = uint16_t;

	struct SVertex_Pos_Tex
	{
		DirectX::SimpleMath::Vector3 P;
		DirectX::SimpleMath::Vector2 T;

		static const int NumInputElements = 2;
		static const D3D11_INPUT_ELEMENT_DESC InputElements[NumInputElements];
	};
	static_assert(sizeof(SVertex_Pos_Tex) == 20, "");

	struct SVertex_Pos_Norm_Tex
	{
		DirectX::SimpleMath::Vector3 P;
		DirectX::SimpleMath::Vector3 N;
		DirectX::SimpleMath::Vector2 T;

		static const int NumInputElements = 3;
		static const D3D11_INPUT_ELEMENT_DESC InputElements[NumInputElements];
	};
	static_assert(sizeof(SVertex_Pos_Norm_Tex) == 32, "");

	class UMesh
	{
	public:
		void Draw() { }

		void BuildDrawItems(eastl::vector<struct SDrawItem>& inOutTargetDrawItems, const SPerDrawConstants& inPerMeshDrawConstants) const;
	private:
		friend class UAssetCache;
		static eastl::shared_ptr<UMesh> Load(const eastl::string& inPath);

		eastl::vector<SSubMesh> m_subMeshes;
		eastl::vector<UMaterial> m_materials;

		eastl::vector<SVertex_Pos_Norm_Tex> m_vertexBuffer;
		eastl::vector<Index_t> m_indexBuffer;

		SBufferId m_gpuVertexBuffer;
		SBufferId m_gpuIndexBuffer;
	};

	struct SMeshInstance
	{
		SMeshInstance() : m_bVisible(false) {}

		SPerDrawConstants m_perDrawConstants;
		eastl::shared_ptr<UMesh> m_mesh;
		bool m_bVisible;
	};
}
