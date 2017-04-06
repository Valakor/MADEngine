#pragma once

#include <EASTL/functional.h>
#include <EASTL/numeric_limits.h>

#include "Misc/Assert.h"

#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

namespace MAD
{
	/*
		UGraphicsObject - Wrapper around the ComPtr<T> for a generic ID3D11DeviceChild type. Currently being used as a wrapper around
		ComPtr<T>s for common render pipeline resources.

		**NECESSARY** because eastl::vector will try to take the address of it's type, which is overloaded to return a reference in the ComPtr implementation.
		This wrapper allows us to have eastl::vector<ComPtr<T>> because the UGraphicsObject class doesn't overload the operator& function to return a reference
		like ComPtr does
	*/
	template <typename T>
	struct UGraphicsObject
	{
		static_assert(eastl::is_base_of<ID3D11DeviceChild, T>::value, "Must be derived from a D3D11 interface");

		ComPtr<T> p;

		UGraphicsObject(T* inObjectPtr = nullptr) { p = inObjectPtr; }

		void Debug_SetName(const eastl::string& inName)
		{
			auto obj = static_cast<ID3D11DeviceChild*>(p.Get());

			obj->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(inName.size()), inName.data());
		}

		// Utility wrapper functions around the ComPtr (most used)
		T* Get() { return p.Get(); }
		T** GetAddressOf() { return p.GetAddressOf(); }
		void Reset() { p.Reset(); }

		// Utility operator overloads around the ComPtr for standardized use of pointer syntax
		UGraphicsObject<T>& operator=(const UGraphicsObject<T>& inOtherObject)
		{
			p = inOtherObject.p;

			return *this;
		}

		ComPtr<T> operator->() { return p; }
		ComPtr<const T> operator->() const { return p; }

		bool operator==(const UGraphicsObject<T>& inOtherObj) const
		{
			return p == inOtherObj.p;
		}

		bool operator==(const T* inOtherObjectPtr) const
		{
			return p.Get() == inOtherObjectPtr;
		}

		bool operator!=(const UGraphicsObject<T>& inOtherObj) const
		{
			return !(*this == inOtherObj);
		}

		bool operator!=(const T* inOtherObjectPtr) const
		{
			return !(*this == inOtherObjectPtr);
		}

		operator bool() const
		{
			return p != nullptr;
		}
	};


	using VertexShaderPtr_t = UGraphicsObject<ID3D11VertexShader>;
	using GeometryShaderPtr_t = UGraphicsObject<ID3D11GeometryShader>;
	using PixelShaderPtr_t = UGraphicsObject<ID3D11PixelShader>;
	using InputLayoutPtr_t = UGraphicsObject<ID3D11InputLayout>;
	using RenderTargetPtr_t = UGraphicsObject<ID3D11RenderTargetView>;
	using DepthStencilPtr_t = UGraphicsObject<ID3D11DepthStencilView>;
	using DepthStencilStatePtr_t = UGraphicsObject<ID3D11DepthStencilState>;
	using BlendStatePtr_t = UGraphicsObject<ID3D11BlendState1>;
	using SamplerStatePtr_t = UGraphicsObject<ID3D11SamplerState>;
	using RasterizerStatePtr_t = UGraphicsObject<ID3D11RasterizerState1>;
	using ShaderResourcePtr_t = UGraphicsObject<ID3D11ShaderResourceView>;
	using BufferPtr_t = UGraphicsObject<ID3D11Buffer>;
	using Texture2DPtr_t = UGraphicsObject<ID3D11Texture2D>;
	using ResourcePtr_t = UGraphicsObject<ID3D11Resource>;
}
