#pragma once

#include <EASTL/shared_ptr.h>
#include <EASTL/string.h>

#include "Rendering/GraphicsDriverTypes.h"

namespace MAD
{
	class URenderPassProgram
	{
	public:
		void SetProgramActive(class UGraphicsDriver& inGraphicsDriver) const;
	private:
		friend class UAssetCache;
		static eastl::shared_ptr<URenderPassProgram> Load(const eastl::string& inPath);
	private:
		// For now, we're only going to use a program with (potentially) only a vertex shader and pixel shader
		// Will probably support an additional geometry shader
		
		SVertexShaderId m_vertexShader;
		SPixelShaderId m_pixelShader;

		// TODO: For now, each program will have an expected input layout and our assets just have to conform to it.
		// Strong chance that we'll change this in the future
		SInputLayoutId m_programInputLayout;
	};
}
