#pragma once

#include "Rendering/TextureCube.h"

namespace MAD
{
	// Only supports loading vertical cross cube maps
	class UColorTextureCube : public UTextureCube
	{
	public:
		// Two methods of creating color texture cube:
		// 1) load from pre-created cube map file
		// 2) create backing texture to render to (TODO)
		UColorTextureCube();
		explicit UColorTextureCube(uint16_t inTexSideRes);
		explicit UColorTextureCube(const eastl::string& inTexturePath);

		virtual void BindCubeSideAsTarget(uint8_t inCubeSide) override;

		void SetClearColor(const Color& inCubeTexClearColor) { m_clearColor = inCubeTexClearColor; }
		const Color& GetClearColor() const { return m_clearColor; }
	private:
		bool m_bIsDynamic;
		Color m_clearColor;
	};
}
