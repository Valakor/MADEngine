#pragma once

#define WIN32_LEAN_AND_MEAN
#include <d3d11_2.h>
#include <wrl/client.h>

#include <EASTL/string.h>
#include <EASTL/shared_ptr.h>

namespace MAD
{
	class UTexture
	{
	public:
		static eastl::shared_ptr<UTexture> Load(const eastl::string& inPath);

		UTexture();

	private:
		uint64_t m_width;
		uint64_t m_height;

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_textureSRV;
	};
}
