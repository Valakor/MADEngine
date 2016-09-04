#pragma once

#define WIN32_LEAN_AND_MEAN
#include <d3d11_2.h>
__pragma(warning(push))
__pragma(warning(disable:4838))
#include <DirectXTK/SimpleMath.h>
__pragma(warning(pop))
#include <wrl/client.h>

#include <EASTL/shared_ptr.h>
#include <EASTL/string.h>
#include <EASTL/vector.h>

#include "Rendering/Material.h"
#include "Rendering/SubMesh.h"

namespace MAD
{
	using Index_t = uint16_t;

	struct SVertex_Pos_Norm_Tex
	{
		DirectX::SimpleMath::Vector3 P;
		DirectX::SimpleMath::Vector3 N;
		DirectX::SimpleMath::Vector2 T;
	};
	static_assert(sizeof(SVertex_Pos_Norm_Tex) == 32, "");

	class UMesh
	{
	public:
		static eastl::shared_ptr<UMesh> Load(const eastl::string& inPath);

	private:
		eastl::vector<SSubMesh> m_subMeshes;
		eastl::vector<UMaterial> m_materials;

		eastl::vector<SVertex_Pos_Norm_Tex> m_vertexBuffer;
		eastl::vector<Index_t> m_indexBuffer;

		Microsoft::WRL::ComPtr<ID3D11Buffer> m_gpuVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_gpuIndexBuffer;
	};
}
