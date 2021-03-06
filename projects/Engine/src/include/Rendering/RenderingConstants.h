#pragma once

#include <EASTL/string.h>

namespace MAD
{
	namespace RenderConstants
	{
		extern const uint32_t DynamicEnvironmentMapRes;
		extern const uint32_t ShadowMapRes;
	}

	namespace ShaderPaths
	{
		extern const eastl::string ReflectionPass;
		extern const eastl::string GBufferPass;
		extern const eastl::string DeferredLightingPass;
		extern const eastl::string SkySpherePass;
		extern const eastl::string DepthPass;
		extern const eastl::string BackBufferFinalizePass;
		extern const eastl::string TextureBlitPass;
		extern const eastl::string DebugGeometryPass;
		extern const eastl::string DebugTextPass;
	}

	namespace TexturePaths
	{
		extern const eastl::string EnvironmentMapTexture;
	}
}
