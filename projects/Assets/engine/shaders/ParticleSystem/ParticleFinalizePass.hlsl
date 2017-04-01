#include "ParticleSystemCommon.hlsl"

// Output of VS
struct ParticleVertexVSOut
{
	float3 PositionVS    : POSITION;
	float2 ParticleSize  : SIZE;
	float4 ParticleColor : COLOR;
	uint   Type			 : TYPE;
};

// Output of GS
struct ParticleVertexGSOut
{
	float4 PositionHS : SV_Position;
	float4 ParticleColor : COLOR;
	//float2 GeometryUV : TEXCOORD; TODO: Use custom texture
};

ParticleVertexVSOut VS(ParticleVertex vsIn)
{
	return vsIn;
}

[maxvertexcount(4)]
void GS(point ParticleVertexVSOut gsIn[1], inout TriangleStream<ParticleVertexGSOut> pVertexOutStream)
{
	// Using a particle
}
