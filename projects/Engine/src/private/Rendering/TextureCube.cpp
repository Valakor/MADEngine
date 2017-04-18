#include "Rendering/TextureCube.h"
#include "Rendering/RenderContext.h"
#include "Rendering/GraphicsDriver.h"

namespace MAD
{
	void UTextureCube::BindAsResource(ETextureSlot inTextureSlot)
	{
		auto& graphicsDriver = URenderContext::Get().GetGraphicsDriver();

		// If we're binding the texture cube as a shader resource, we must unbind the depth stencil view
		graphicsDriver.SetRenderTargets(nullptr, 0, nullptr);
		graphicsDriver.SetPixelShaderResource(m_textureCubeSRV, inTextureSlot);
	}
}
