#pragma once

#include <EASTL/shared_ptr.h>
#include <EASTL/string.h>

#include "Rendering/GraphicsDriverTypes.h"

namespace MAD
{
	using ProgramId_t = uint64_t;

	enum class EProgramIdMask : ProgramId_t
	{
		EProgramIdMask_Diffuse = 1 << 0,
		EProgramIdMask_Specular = 1 << 1,
		EProgramIdMask_Emissive = 1 << 2,
		EProgramIdMask_NormalMap = 1 << 3,
		EProgramIdMask_Invalid
	};

	class URenderPassProgram
	{
	public:
		static EProgramIdMask ConvertStringToPIDMask(const char* inMaskString);
		static const char* ConvertPIDMaskToString(EProgramIdMask inMaskId);
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
	};
}
