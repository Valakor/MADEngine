#pragma once

#define WIN32_LEAN_AND_MEAN
#include <d3d11_1.h>
__pragma(warning(push))
__pragma(warning(disable:4838))
#include <DirectXTK/SimpleMath.h>
__pragma(warning(pop))

#include "Rendering/Texture.h"

namespace MAD
{
	struct SGpuMaterial
	{
		DirectX::SimpleMath::Vector3 m_diffuseColor;

	private:
		float __pad1 = 0.0f;

		// 16 bytes ---------------------------------

	public:
		DirectX::SimpleMath::Vector3 m_specularColor;
		float m_specularPower = 1.0f;

		// 16 bytes ---------------------------------

		DirectX::SimpleMath::Vector3 m_emissiveColor;

	private:
		float __pad2 = 0.0f;

		// 16 bytes ---------------------------------

	public:
		BOOL m_bHasDiffuseTex = FALSE;
		BOOL m_bHasSpecularTex = FALSE;
		BOOL m_bHasEmissiveTex = FALSE;

	private:
		float __pad3 = 0.0f;

		// 16 bytes ---------------------------------
	};
	static_assert(sizeof(SGpuMaterial) == 64, "");

	class UMaterial
	{
	public:
		UMaterial();

		SGpuMaterial m_mat;

		UTexture m_diffuseTex;
		UTexture m_specularTex;
		UTexture m_emissiveTex;
	};
}
