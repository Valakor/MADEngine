#include "Common.hlsl"

//=>:(Usage, VS, vs_5_0)
//=>:(Usage, PS, ps_5_0)

// Input structs for vertex and pixel shader
struct VS_INPUT
{
    float3 mPos : POSITION;
};

struct PS_INPUT
{
	float4 mPos : SV_POSITION;
};


//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output;
    output.mPos = float4(input.mPos, 1.0);
    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(PS_INPUT input) : SV_Target
{
    int3 texCoord = int3(input.mPos.xy, 0);
	
    float depth = g_diffuseBuffer.Load(texCoord).r;
    float depthLinear = 2.0 * g_cameraNearPlane * g_cameraFarPlane / (g_cameraFarPlane + g_cameraNearPlane - depth * (g_cameraFarPlane - g_cameraNearPlane));
    float depthLinearNormalized = (depthLinear - g_cameraNearPlane) / g_cameraFarPlane;

    return float4(depthLinearNormalized, depthLinearNormalized, depthLinearNormalized, 1.0);
}
