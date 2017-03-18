#include "..\Common.hlsl"
#include "ParticleSystemCommon.hlsl"

//=>:(Usage, VS, vs_5_0)
//=>:(Usage, PS, ps_5_0)
//=>:(Usage, GS, gs_5_0)

// Output of VS
struct ParticleVSOut
{
	float3 PositionVS    : POSITION;
	float3 VelocityVS	 : VELOCITY;
	float4 ParticleColor : COLOR;
};

// Output of GS
struct ParticleVertexGSOut
{
	float4 PositionHS : SV_Position;
	float4 ParticleColor : COLOR;
};

ParticleVSOut VS(Particle vsIn)
{
	ParticleVSOut vsOut;

	const float3 gravitationalAccel = { 0.0f, 90.8f, 0.0f };
	const float t = vsIn.Age;

	vsOut.PositionVS = vsIn.InitialPosVS + (vsIn.InitialVelVS * t) + (-0.5f * gravitationalAccel * t * t);
	vsOut.VelocityVS = (vsIn.InitialVelVS - gravitationalAccel * t) * 0.1;
	vsOut.ParticleColor = vsIn.ParticleColor;

	return vsOut;
}

[maxvertexcount(2)]
void GS(point ParticleVSOut gsIn[1], inout LineStream<ParticleVertexGSOut> pVertexOutStream)
{
	// Expand the particle point into camera facing quad with appropriate color/UV
	// Particle is already in View Space so we can just add scalars to the x and y direction since its aligned to the camera already
	ParticleVertexGSOut beginLineVert;
	ParticleVertexGSOut endLineVert;

	beginLineVert.PositionHS = mul(float4(gsIn[0].PositionVS, 1.0f), g_cameraProjectionMatrix);
	beginLineVert.ParticleColor = gsIn[0].ParticleColor;

	endLineVert.PositionHS = mul(float4(gsIn[0].PositionVS + gsIn[0].VelocityVS, 1.0f), g_cameraProjectionMatrix);
	endLineVert.ParticleColor = gsIn[0].ParticleColor;

	pVertexOutStream.Append(beginLineVert);
	pVertexOutStream.Append(endLineVert);
}

float4 PS(ParticleVertexGSOut inGSOut) : SV_Target
{
	return inGSOut.ParticleColor;
}