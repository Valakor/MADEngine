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
		{ "TEXCOORD", EInputLayoutSemantic::UV       },
		{ "TANGENT",  EInputLayoutSemantic::Tangent  },
	};

	eastl::hash_map<InputLayoutFlags_t, SInputLayoutId> UInputLayoutCache::s_inputLayoutCache;

	bool UInputLayoutCache::RegisterInputLayout(UGraphicsDriver& inGraphicsDriver, InputLayoutFlags_t inFlags, const eastl::vector<char>& inCompiledVertexShader)
	{
		return RegisterInputLayout(inGraphicsDriver, inFlags, inCompiledVertexShader.data(), inCompiledVertexShader.size());
	}

	bool UInputLayoutCache::RegisterInputLayout(UGraphicsDriver& inGraphicsDriver, InputLayoutFlags_t inFlags, const void* inCompiledVSByteCode, size_t inByteCodeSize)
	{
		MAD_ASSERT_DESC(inCompiledVSByteCode != nullptr && inByteCodeSize > 0, "Invalid parameters to RegisterInputLayout");
		MAD_ASSERT_DESC(inFlags != EInputLayoutSemantic::INVALID, "Invalid flags passed to RegisterInputLayout");

		auto layoutId = TryGetInputLayout(inFlags);
		if (layoutId != SInputLayoutId::Invalid)
		{
			return true;
		}

		layoutId = CreateInputLayout(inGraphicsDriver, inFlags, inCompiledVSByteCode, inByteCodeSize);
		MAD_ASSERT_DESC(layoutId != SInputLayoutId::Invalid, "Failed to create input layout from given flags");

		s_inputLayoutCache[inFlags] = layoutId;
		return layoutId != SInputLayoutId::Invalid;
	}

	SInputLayoutId UInputLayoutCache::CreateInputLayout(UGraphicsDriver& inGraphicsDriver, InputLayoutFlags_t inFlags, const void* inCompiledVSByteCode, size_t inByteCodeSize)
	{
		static const D3D11_INPUT_ELEMENT_DESC position = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, AsIntegral(EVertexBufferSlot::Position), 0, D3D11_INPUT_PER_VERTEX_DATA, 0 };
		static const D3D11_INPUT_ELEMENT_DESC normal   = { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, AsIntegral(EVertexBufferSlot::Normal),   0, D3D11_INPUT_PER_VERTEX_DATA, 0 };
		static const D3D11_INPUT_ELEMENT_DESC texcoord = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    AsIntegral(EVertexBufferSlot::UV),       0, D3D11_INPUT_PER_VERTEX_DATA, 0 };
		static const D3D11_INPUT_ELEMENT_DESC tangent  = { "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, AsIntegral(EVertexBufferSlot::Tangent),  0, D3D11_INPUT_PER_VERTEX_DATA, 0 };

		eastl::vector<D3D11_INPUT_ELEMENT_DESC> inputLayout;

		if (inFlags & EInputLayoutSemantic::Position)
		{
			inputLayout.push_back(position);
		}

		if (inFlags & EInputLayoutSemantic::Normal)
		{
			inputLayout.push_back(normal);
		}

		if (inFlags & EInputLayoutSemantic::UV)
		{
			inputLayout.push_back(texcoord);
		}

		if (inFlags & EInputLayoutSemantic::Tangent)
		{
			inputLayout.push_back(tangent);
		}

		return inGraphicsDriver.CreateInputLayout(inputLayout.data(), static_cast<UINT>(inputLayout.size()), inCompiledVSByteCode, inByteCodeSize);
	}

	SInputLayoutId UInputLayoutCache::GetInputLayout(InputLayoutFlags_t inFlags)
	{
		auto layoutId = TryGetInputLayout(inFlags);

		MAD_ASSERT_DESC(layoutId != SInputLayoutId::Invalid, "Attempting to retrieve an input layout that does not exist yet");
		return layoutId;
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

	SInputLayoutId UInputLayoutCache::TryGetInputLayout(InputLayoutFlags_t inFlags)
	{
		auto iter = s_inputLayoutCache.find(inFlags);
		if (iter != s_inputLayoutCache.end())
		{
			return iter->second;
		}

		return SInputLayoutId::Invalid;
	}
}
