#pragma once

#define WIN32_LEAN_AND_MEAN
#include <d3d11_2.h>
__pragma(warning(push))
__pragma(warning(disable:4838))
#include <DirectXTK/SimpleMath.h>
__pragma(warning(pop))

namespace MAD
{
	inline constexpr float ConvertToRadians(float inDegrees)
	{
		return inDegrees * (DirectX::XM_PI / 180.0f);
	}

	inline constexpr float ConvertToDegrees(float inRadians)
	{
		return inRadians * (180.0f / DirectX::XM_PI);
	}

}