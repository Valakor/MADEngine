#include "Common.hlsl"

// Input structs for vertex and pixel shader
struct PS_INPUT
{
	float4 mPos : SV_POSITION;
};


//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS(uint id : SV_VertexID)
{
	PS_INPUT output;

	float2 texCoord = float2(id & 1, id >> 1);
	output.mPos = float4((texCoord.x - 0.5f) * 2.0f, -(texCoord.y - 0.5f) * 2.0f, 0.0f, 1.0f);

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
