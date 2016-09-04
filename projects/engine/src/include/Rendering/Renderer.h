#pragma once

#define WIN32_LEAN_AND_MEAN
#include <d3d11_2.h>
#include <wrl/client.h>

#include <EASTL/shared_ptr.h>
#include <EASTL/string.h>

class UTexture;

namespace MAD
{
	class URenderer
	{
	public:
		URenderer();

		bool Init();
		void Shutdown();

		void Frame(float framePercent);

		void OnScreenSizeChanged();

		// Don't use this directly, use the AssetCache interface, e.g.
		//     AssetCache.Load<UTexture>(...);
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CreateTextureFromFile(const eastl::string& inPath, uint64_t& outWidth, uint64_t& outHeight) const;

	private:
		void BeginFrame();
		void Draw();
		void EndFrame();
	};
}
