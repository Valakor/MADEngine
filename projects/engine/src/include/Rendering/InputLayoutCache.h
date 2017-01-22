#pragma once

#include <EASTL/hash_map.h>
#include <EASTL/string.h>
#include <EASTL/vector.h>

#include "Rendering/GraphicsDriverTypes.h"

namespace MAD
{
	using InputLayoutFlags_t = uint8_t;

	namespace EInputLayoutSemantic
	{
		enum Type : InputLayoutFlags_t
		{
			Position	= 1 << 0,
			Normal		= 1 << 1,
			Tangent		= 1 << 2,
			UV			= 1 << 3,

			INVALID		= 0
		};
	}

	class UInputLayoutCache
	{
	public:
		static bool RegisterInputLayout(class UGraphicsDriver& inGraphicsDriver, InputLayoutFlags_t inFlags, const eastl::vector<char>& inCompiledVertexShader);
		static bool RegisterInputLayout(class UGraphicsDriver& inGraphicsDriver, InputLayoutFlags_t inFlags, const void* inCompiledVSByteCode, size_t inByteCodeSize);

		static InputLayoutPtr_t GetInputLayout(InputLayoutFlags_t inFlags);

		static InputLayoutFlags_t GetFlagForSemanticName(const eastl::string& inSemanticName);

	private:
		static const eastl::hash_map<eastl::string, InputLayoutFlags_t> s_semanticNameToFlagMap;
		static eastl::hash_map<InputLayoutFlags_t, InputLayoutPtr_t> s_inputLayoutCache;

		static InputLayoutPtr_t TryGetInputLayout(InputLayoutFlags_t inFlags);
		static InputLayoutPtr_t CreateInputLayout(class UGraphicsDriver& inGraphicsDriver, InputLayoutFlags_t inFlags, const void* inCompiledVSByteCode, size_t inByteCodeSize);
	};
}
