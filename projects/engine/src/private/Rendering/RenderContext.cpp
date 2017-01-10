#include "stdafx.h"
#include "Rendering/RenderContext.h"

#include "Core/GameEngine.h"

#include "Rendering/GraphicsDriver.h"
#include "Rendering/Renderer.h"

namespace MAD
{
	UGraphicsDriver& URenderContext::GetGraphicsDriver()
	{
		return GetRenderer().GetGraphicsDriver();
	}

	URenderer& URenderContext::GetRenderer()
	{
		MAD_ASSERT_DESC(gEngine != nullptr, "Error: The renderer cannot be accessed before the game engine is initialized!");

		return gEngine->GetRenderer();
	}

}