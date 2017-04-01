#include "Rendering/InputLayoutElementArray.h"

namespace MAD
{
	const D3D11_INPUT_CLASSIFICATION UInputLayoutElementArray::s_inputClassification = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA;

	SInputLayoutElement::SInputLayoutElement(const eastl::string& inName, uint32_t inIndex, DXGI_FORMAT inFormat, uint32_t inSlot, uint32_t inOffset, D3D11_INPUT_CLASSIFICATION inClass, uint32_t inStepRate)
		: SemanticName(inName)
		, SemanticIndex(inIndex)
		, Format(inFormat)
		, InputSlot(inSlot)
		, ByteOffset(inOffset)
		, InputSlotClass(inClass)
		, StepRate(inStepRate) {}

	UInputLayoutElementArray::UInputLayoutElementArray() : m_layoutFlags(0) {}

	bool UInputLayoutElementArray::operator==(const UInputLayoutElementArray& inElemArray) const
	{
		return eastl::hash<UInputLayoutElementArray>()(*this) == eastl::hash<UInputLayoutElementArray>()(inElemArray);
	}

	void UInputLayoutElementArray::Push(const eastl::string& inSemanticName, uint32_t inIndex, DXGI_FORMAT inFormat, uint32_t inSlot)
	{
		m_layoutFlags |= inSlot;
		m_layoutElements.emplace_back(inSemanticName, inIndex, inFormat, inSlot, 0, UInputLayoutElementArray::s_inputClassification, 0);
	}
}