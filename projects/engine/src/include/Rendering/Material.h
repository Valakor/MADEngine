#pragma once

#include "Rendering/RenderingCommon.h"
#include "Rendering/Texture.h"

namespace MAD
{
	class UMaterial
	{
	public:
		UMaterial();

		SGPUMaterial m_mat;

		UTexture m_diffuseTex;
		UTexture m_specularTex;
		UTexture m_emissiveTex;
		UTexture m_normalMap;
		UTexture m_opacityMask;

		bool m_isTwoSided = false;
	};
}
