#include "Rendering/RenderingConstants.h"

namespace MAD
{
	namespace AssetPaths
	{
		const eastl::string GBufferPass = "engine\\shaders\\GBuffer.hlsl";
		const eastl::string DeferredLightingPass = "engine\\shaders\\DeferredLighting.hlsl";
		const eastl::string DepthPass = "engine\\shaders\\RenderGeometryToDepth.hlsl";
		const eastl::string BackBufferFinalizePass = "engine\\shaders\\BackBufferFinalize.hlsl";
		const eastl::string TextureBlitPass = "engine\\shaders\\CopyTexture.hlsl";
		const eastl::string DebugGeometryPass = "engine\\shaders\\RenderDebugPrimitives.hlsl";
		const eastl::string DebugTextPass = "engine\\shaders\\RenderDebugText.hlsl";
	}
}