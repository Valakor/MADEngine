#include "Rendering/TextureCube.h"
#include "Rendering/GraphicsDriver.h"

namespace MAD
{

	UTextureCube::UTextureCube(uint16_t inTextureRes) : m_cubeTextureResolution(inTextureRes)
	{
		// POSITIVE X = 0
		// NEGATIVE X = 1
		// POSITIVE Y = 2
		// NEGATIVE Y = 3
		// POSITIVE Z = 4
		// NEGATIVE Z = 5
	}

	void UTextureCube::BindCubeSideAsTarget(int inCubeSide)
	{
		UNREFERENCED_PARAMETER(inCubeSide);
	}

	void UTextureCube::BindAsResource(int inTextureSlot)
	{
		UNREFERENCED_PARAMETER(inTextureSlot);
	}
}