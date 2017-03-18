#include "Rendering/InputLayout.h"
#include "Rendering/RenderContext.h"
#include "Rendering/GraphicsDriver.h"

namespace MAD
{
	UInputLayout::UInputLayout(const UInputLayoutElementArray& inLayoutElements, const void* inCompiledVSByteCode, size_t inByteCodeSize) : m_elementArray(inLayoutElements)
	{
		// Create the input layout using the element array
		const auto& inputElements = m_elementArray.GetLayoutElements();
		eastl::vector<D3D11_INPUT_ELEMENT_DESC> d3d11InputElements;

		for (const auto& curElem : inputElements)
		{
			d3d11InputElements.emplace_back(D3D11_INPUT_ELEMENT_DESC{ curElem.SemanticName.c_str(), curElem.SemanticIndex, curElem.Format, curElem.InputSlot, curElem.ByteOffset, curElem.InputSlotClass, curElem.StepRate });
		}

		m_inputLayout = URenderContext::Get().GetGraphicsDriver().CreateInputLayout(d3d11InputElements.data(), static_cast<UINT>(d3d11InputElements.size()), inCompiledVSByteCode, inByteCodeSize);
	}

	void UInputLayout::BindToPipeline()
	{
		URenderContext::Get().GetGraphicsDriver().SetInputLayout(m_inputLayout);
	}

	uint32_t UInputLayout::GetFlagForSemanticName(const eastl::string& inSemanticName) const
	{
		const auto& inputElements = m_elementArray.GetLayoutElements();
		for (const auto& currentElement : inputElements)
		{
			if (inSemanticName == currentElement.SemanticName)
			{
				return currentElement.InputSlot;
			}
		}

		return 0; // Invalid
	}
}
