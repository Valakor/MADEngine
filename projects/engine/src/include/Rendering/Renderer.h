#pragma once

#define WIN32_LEAN_AND_MEAN
#include <d3d11_2.h>
#include <wrl/client.h>

#include <EASTL/shared_ptr.h>
#include <EASTL/string.h>

namespace MAD
{
	class UTexture;
	class UGameWindow;

	class URenderer
	{
	public:
		URenderer();

		bool Init(UGameWindow& inWindow);
		void Shutdown();

		void Frame(float framePercent);

		void OnScreenSizeChanged();

		// Don't use this directly, use the AssetCache interface, e.g.
		//     AssetCache.Load<UTexture>(...);
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CreateTextureFromFile(const eastl::string& inPath, uint64_t& outWidth, uint64_t& outHeight) const;

		Microsoft::WRL::ComPtr<ID3D11Buffer> CreateBuffer(const void* inData, UINT inDataSize, D3D11_USAGE inUsage, UINT inBindFlags, UINT inCpuAccessFlags) const;
		Microsoft::WRL::ComPtr<ID3D11Buffer> CreateVertexBuffer(const void* inData, UINT inDataSize) const;
		Microsoft::WRL::ComPtr<ID3D11Buffer> CreateIndexBuffer(const void* inData, UINT inDataSize) const;
		Microsoft::WRL::ComPtr<ID3D11Buffer> CreateConstantBuffer(const void* inData, UINT inDataSize) const;

		template <typename T>
		T* MapBuffer(Microsoft::WRL::ComPtr<ID3D11Buffer> inBuffer) const;
		void* MapBuffer(Microsoft::WRL::ComPtr<ID3D11Buffer> inBuffer) const;
		void UnmapBuffer(Microsoft::WRL::ComPtr<ID3D11Buffer> inBuffer) const;

		template <typename T>
		void UpdateBuffer(Microsoft::WRL::ComPtr<ID3D11Buffer> inBuffer, const T& inData) const;
		void UpdateBuffer(Microsoft::WRL::ComPtr<ID3D11Buffer> inBuffer, const void* inData, size_t inDataSize) const;

		void SetViewport(float inWidth, float inHeight) const;
		void SetViewport(POINT inDimensions) const;

		void SetFullScreen(bool inIsFullscreen) const;

	private:
		void BeginFrame();
		void Draw();
		void EndFrame();

		UGameWindow* m_window;
	};

	template <typename T>
	T* URenderer::MapBuffer(Microsoft::WRL::ComPtr<ID3D11Buffer> inBuffer) const
	{
		return reinterpret_cast<T*>(MapBuffer(inBuffer));
	}

	template <typename T>
	void URenderer::UpdateBuffer(Microsoft::WRL::ComPtr<ID3D11Buffer> inBuffer, const T& inData) const
	{
		auto data = MapBuffer<T>(inBuffer);
		*data = inData;
		UnmapBuffer(inBuffer);
	}
}
