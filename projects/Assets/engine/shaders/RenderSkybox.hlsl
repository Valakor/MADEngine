#include "Common.hlsl"

//=>:(Usage, VS, vs_5_0)
//=>:(Usage, PS, ps_5_0)

struct VS_INPUT
{
	float3 PositionMS : POSITION;
};

struct VS_OUTPUT
{
	float4 PositionCS : SV_POSITION;
	float3 TexCoord : TEXCOORD;
};

VS_OUTPUT VS(VS_INPUT vsInput)
{
	VS_OUTPUT vsOutput;

	float3 skyboxPosScaled = mul(float4(vsInput.PositionMS, 1.0), g_objectToWorldMatrix).xyz;
	vsOutput.PositionCS = mul(float4(skyboxPosScaled, 1.0), g_cameraProjectionMatrix);
	vsOutput.TexCoord = mul(float4(skyboxPosScaled, 0.0), g_cameraInverseViewMatrix); // We only want the sky sphere to rotate around the camera, not translate with it
	
	return vsOutput;
}

float4 PS(VS_OUTPUT vsOutput) : SV_Target
{
	return g_cubeMap.Sample(g_pointSampler, vsOutput.TexCoord);
}