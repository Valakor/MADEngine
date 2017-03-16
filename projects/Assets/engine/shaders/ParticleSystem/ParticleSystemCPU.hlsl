#include "ParticleSystemCommon.hlsl"

// Output of VS
struct ParticleVSOut
{
	float3 PositionVS    : POSITION;
	float2 ParticleSize  : SIZE;
	float4 ParticleColor : COLOR;
	//uint   Type			 : TYPE;
};

// Output of GS
struct ParticleVertexGSOut
{
	float4 PositionHS : SV_Position;
	float4 ParticleColor : COLOR;
};

ParticleVSOut VS(Particle vsIn)
{
	const float3 gravitationalAccel = { 0.0f, 7.8f, 0.0f };
	const float gt = g_gameTime;

	ParticleVSOut vsOut;
	
	vsOut.PositionVS = vsIn.InitialPosVS + (vsIn.InitialVelVS * gt) + (-0.5f * gravitationalAccel * gt * gt);
	vsOut.ParticleSize = vsIn.ParticleSize;
	vsOut.Type = vsIn.Type;

	return vsIn;
}

[maxvertexcount(4)]
void GS(point ParticleVSOut gsIn[1], inout TriangleStream<ParticleVertexGSOut> pVertexOutStream)
{
	// Expand the particle point into camera facing quad with appropriate color/UV
	// Particle is already in View Space so we can just add scalars to the x and y direction since its aligned to the camera already
	ParticleVertexGSOut billboardVertices[4];
	
	float3x3 lookAtMatrix = CreateLookAt(gsIn[0].PositionVS, float3(0.0f, 0.0f, 0.0f), float3(0.0f, 1.0f, 0.0f));
	float3 basisVectors[4] = 
	{
		{ -1.0f,  1.0f , 0.0f }, // Top-left
		{ 1.0f ,  1.0f , 0.0f }, // Top-right
		{ -1.0f, -1.0f , 0.0f }, // Bottom-left
		{ 1.0f , -1.0f , 0.0f }  // Bottom-right
	};

	for (int i = 0; i < 4; ++i)
	{
		billboardVertHSPos[i].PositionHS = mul(float4(gsIn[0].PositionVS + basisVectors[i] * float3(gsIn[0].ParticleSize / 2.0f, 0.0f)), g_projectionMatrix);
		billboardVertices[i].ParticleColor = gsIn[0].ParticleColor;
		pVertexOutStream.Append(billboardVertices[i]);
	}
}

float4 PS(ParticleVertexGSOut inGSOut) : SV_Target
{
	return inGSOut.ParticleColor;
}