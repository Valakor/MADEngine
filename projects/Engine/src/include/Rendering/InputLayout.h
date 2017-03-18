#pragma once

#include <EASTL/vector.h>
#include <EASTL/string.h>

#include "Rendering/GraphicsDriverTypes.h"
#include "Rendering/InputLayoutElementArray.h"

namespace MAD
{
	// TODO In progress, eventually try to integrate into engine, but too messy currently
	using InputLayoutSlots_t = uint32_t;

	class UInputLayout
	{
	public:
		UInputLayout(const UInputLayoutElementArray& inLayoutElements, const void* inCompiledVSByteCode, size_t inByteCodeSize);

		void BindToPipeline();

		const UInputLayoutElementArray& GetElementLayout() const { return m_elementArray; }
		InputLayoutSlots_t GetInputLayoutFlags() const { return m_elementArray.GetLayoutSlotFlags(); }
		InputLayoutSlots_t GetFlagForSemanticName(const eastl::string& inSemanticName) const;
	private:
		UInputLayoutElementArray m_elementArray;

		InputLayoutPtr_t m_inputLayout;
	};
}