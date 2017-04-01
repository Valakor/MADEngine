#pragma once

#include "Core/SimpleMath.h"
#include "Misc/Assert.h"

#include <EASTL/vector.h>
#include <EASTL/string.h>

namespace MAD
{
	// TODO In progress, eventually try to integrate into engine, but too messy currently

	struct SInputLayoutElement
	{
		SInputLayoutElement(const eastl::string& inName, uint32_t inIndex, DXGI_FORMAT inFormat,
							uint32_t inSlot, uint32_t inOffset, D3D11_INPUT_CLASSIFICATION inClass, uint32_t inStepRate);
		eastl::string				SemanticName;
		uint32_t					SemanticIndex;
		DXGI_FORMAT					Format;
		uint32_t					InputSlot;
		uint32_t					ByteOffset;
		D3D11_INPUT_CLASSIFICATION	InputSlotClass;
		uint32_t					StepRate;
	};

	class UInputLayoutElementArray
	{
	public:
		UInputLayoutElementArray();

		template <typename ElementType>
		void PushElement(const eastl::string&, uint32_t, uint32_t)
		{
			MAD_ASSERT_DESC(false, "Unknown type!");
		}
		
		template <>
		void PushElement<float>(const eastl::string& inSemanticName, uint32_t inIndex, uint32_t inSlot)
		{
			Push(inSemanticName, inIndex, DXGI_FORMAT_R32_FLOAT, inSlot);
		}

		template <>
		void PushElement<Vector3>(const eastl::string& inSemanticName, uint32_t inIndex, uint32_t inSlot)
		{
			Push(inSemanticName, inIndex, DXGI_FORMAT_R32G32B32_FLOAT, inSlot);
		}

		template <>
		void PushElement<Vector4>(const eastl::string& inSemanticName, uint32_t inIndex, uint32_t inSlot)
		{
			Push(inSemanticName, inIndex, DXGI_FORMAT_R32G32B32A32_FLOAT, inSlot);
		}

		template <>
		void PushElement<Vector2>(const eastl::string& inSemanticName, uint32_t inIndex, uint32_t inSlot)
		{
			Push(inSemanticName, inIndex, DXGI_FORMAT_R32G32_FLOAT, inSlot);
		}

		uint32_t GetLayoutSlotFlags() const { return m_layoutFlags; }
		const eastl::vector<SInputLayoutElement>& GetLayoutElements() const { return m_layoutElements; }

		bool operator==(const UInputLayoutElementArray& inElemArray) const;
	private:
		void Push(const eastl::string& inSemanticName, uint32_t inIndex, DXGI_FORMAT inFormat, uint32_t inSlot);
	private:
		static const D3D11_INPUT_CLASSIFICATION s_inputClassification;

		uint32_t m_layoutFlags;
		eastl::vector<SInputLayoutElement> m_layoutElements;
	};
}

namespace eastl
{
	template <>
	struct hash<MAD::SInputLayoutElement>
	{
	public:
		size_t operator()(const SInputLayoutElement& inLayoutElem) const
		{
			return   hash<string>()(inLayoutElem.SemanticName)
				   ^ hash<uint32_t>()(inLayoutElem.SemanticIndex)
				   ^ hash<int>()(inLayoutElem.Format)
				   ^ hash<uint32_t>()(inLayoutElem.InputSlot)
				   ^ hash<uint32_t>()(inLayoutElem.ByteOffset)
				   ^ hash<int>()(inLayoutElem.InputSlotClass)
				   ^ hash<uint32_t>()(inLayoutElem.StepRate);
		}
	};

	template <>
	struct hash<MAD::UInputLayoutElementArray>
	{
	public:
		size_t operator()(const UInputLayoutElementArray& inInputElemArray) const
		{
			size_t outputHash = 0;
			const auto& layoutElements = inInputElemArray.GetLayoutElements();

			for (const auto& currLayoutElem : layoutElements)
			{
				outputHash ^= hash<MAD::SInputLayoutElement>()(currLayoutElem);
			}

			return outputHash;
		}
	};
}