#include "Rendering/InputLayoutCache.h"

#include "Misc/Assert.h"
#include "Rendering/GraphicsDriver.h"
#include "Rendering/RenderingCommon.h"

namespace MAD
{
	const eastl::hash_map<eastl::string, InputLayoutFlags_t> UInputLayoutCache::s_semanticNameToFlagMap =
	{
		{ "POSITION", EInputLayoutSemantic::Position },
		{ "NORMAL",   EInputLayoutSemantic::Normal   },
		{ "TANGENT",  EInputLayoutSemantic::Tangent  },
		{ "TEXCOORD", EInputLayoutSemantic::UV       },
	};

	eastl::hash_map<InputLayoutFlags_t, InputLayoutPtr_t> UInputLayoutCache::s_inputLayoutCache;

	bool UInputLayoutCache::RegisterInputLayout(UGraphicsDriver& inGraphicsDriver, InputLayoutFlags_t inFlags, const eastl::vector<char>& inCompiledVertexShader)
	{
		return RegisterInputLayout(inGraphicsDriver, inFlags, inCompiledVertexShader.data(), inCompiledVertexShader.size());
	}

	bool UInputLayoutCache::RegisterInputLayout(UGraphicsDriver& inGraphicsDriver, InputLayoutFlags_t inFlags, const void* inCompiledVSByteCode, size_t inByteCodeSize)
	{
		MAD_ASSERT_DESC(inCompiledVSByteCode != nullptr && inByteCodeSize > 0, "Invalid parameters to RegisterInputLayout");
		MAD_ASSERT_DESC(inFlags != EInputLayoutSemantic::INVALID, "Invalid flags passed to RegisterInputLayout");

		auto layoutPtr = TryGetInputLayout(inFlags);
		if (layoutPtr)
		{
			return true;
		}

		layoutPtr = CreateInputLayout(inGraphicsDriver, inFlags, inCompiledVSByteCode, inByteCodeSize);
		MAD_ASSERT_DESC(layoutPtr, "Failed to create input layout from given flags");

		s_inputLayoutCache[inFlags] = layoutPtr;
		return layoutPtr;
	}

	InputLayoutPtr_t UInputLayoutCache::CreateInputLayout(UGraphicsDriver& inGraphicsDriver, InputLayoutFlags_t inFlags, const void* inCompiledVSByteCode, size_t inByteCodeSize)
	{
		static const D3D11_INPUT_ELEMENT_DESC position = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, AsIntegral(EVertexBufferSlot::Position), 0, D3D11_INPUT_PER_VERTEX_DATA, 0 };
		static const D3D11_INPUT_ELEMENT_DESC normal   = { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, AsIntegral(EVertexBufferSlot::Normal),   0, D3D11_INPUT_PER_VERTEX_DATA, 0 };
		static const D3D11_INPUT_ELEMENT_DESC tangent  = { "TANGENT",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, AsIntegral(EVertexBufferSlot::Tangent),  0, D3D11_INPUT_PER_VERTEX_DATA, 0 };
		static const D3D11_INPUT_ELEMENT_DESC texcoord = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    AsIntegral(EVertexBufferSlot::UV),       0, D3D11_INPUT_PER_VERTEX_DATA, 0 };

		eastl::vector<D3D11_INPUT_ELEMENT_DESC> inputLayout;

		if (inFlags & EInputLayoutSemantic::Position)
		{
			inputLayout.push_back(position);
		}

		if (inFlags & EInputLayoutSemantic::Normal)
		{
			inputLayout.push_back(normal);
		}

		if (inFlags & EInputLayoutSemantic::Tangent)
		{
			inputLayout.push_back(tangent);
		}

		if (inFlags & EInputLayoutSemantic::UV)
		{
			inputLayout.push_back(texcoord);
		}

		return inGraphicsDriver.CreateInputLayout(inputLayout.data(), static_cast<UINT>(inputLayout.size()), inCompiledVSByteCode, inByteCodeSize);
	}

	InputLayoutPtr_t UInputLayoutCache::GetInputLayout(InputLayoutFlags_t inFlags)
	{
		auto layoutPtr = TryGetInputLayout(inFlags);

		MAD_ASSERT_DESC(layoutPtr != nullptr, "Attempting to retrieve an input layout that does not exist yet");
		return layoutPtr;
	}

	InputLayoutFlags_t UInputLayoutCache::GetFlagForSemanticName(const eastl::string& inSemanticName)
	{
		auto iter = s_semanticNameToFlagMap.find(inSemanticName);
		if (iter != s_semanticNameToFlagMap.end())
		{
			return iter->second;
		}

		return EInputLayoutSemantic::INVALID;
	}

	InputLayoutPtr_t UInputLayoutCache::TryGetInputLayout(InputLayoutFlags_t inFlags)
	{
		auto iter = s_inputLayoutCache.find(inFlags);
		if (iter != s_inputLayoutCache.end())
		{
			return iter->second;
		}

		return nullptr;
	}
}
