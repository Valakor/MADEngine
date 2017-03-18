#include "..\Common.hlsl"
#include "ParticleSystemCommon.hlsl"

//=>:(Usage, VS, vs_5_0)
//=>:(Usage, PS, ps_5_0)
//=>:(Usage, GS, gs_5_0)

// Output of VS
struct ParticleVSOut
{
	float3 PositionVS    : POSITION;
	float4 ParticleColor : COLOR;
	float2 ParticleSize  : SIZE;
	//uint   Type			 : TYPE;
};

// Output of GS
struct ParticleVertexGSOut
{
	float4 PositionHS : SV_Position;
	float4 ParticleColor : COLOR;
	float2 ParticleUV    : TEXCOORD;
};

ParticleVSOut VS(Particle vsIn)
{
	ParticleVSOut vsOut;

	const float3 gravitationalAccel = { 0.0f, 7.8f, 0.0f };
	const float t = vsIn.Age;
	float vsDistance = 0.0f;

	vsOut.PositionVS = vsIn.InitialPosVS + (vsIn.InitialVelVS * t) + (-0.5f * gravitationalAccel * t * t);

	vsDistance = distance(vsOut.PositionVS, vsIn.InitialPosVS);

	vsOut.ParticleSize = smoothstep(150.0f, 0.0f, vsDistance) * vsIn.ParticleSize;
	vsOut.ParticleColor = vsIn.ParticleColor;
	//vsOut.Type = vsIn.Type;

	return vsOut;
}

[maxvertexcount(4)]
void GS(point ParticleVSOut gsIn[1], inout TriangleStream<ParticleVertexGSOut> pVertexOutStream)
{
	// Expand the particle point into camera facing quad with appropriate color/UV
	// Particle is already in View Space so we can just add scalars to the x and y direction since its aligned to the camera already
	ParticleVertexGSOut billboardVertices[4];
	
	float3x3 lookAtMatrix = CreateLookAt(gsIn[0].PositionVS, float3(0.0f, 0.0f, 0.0f), float3(0.0f, 1.0f, 0.0f)); //[0] Forward, [1] Left, [2] Up
	
	float3 basisVectors[4] = 
	{
		{ -1.0f,  1.0f , 0.0f }, // Top-left
		{ -1.0f, -1.0f , 0.0f }, // Bottom-left
		{ 1.0f ,  1.0f , 0.0f }, // Top-right
		{ 1.0f , -1.0f , 0.0f }  // Bottom-right
	};

	float2 texCoords[4] =
	{
		{ 0.0f,  1.0f }, // Top-left
		{ 0.0f,  0.0f }, // Bottom-left
		{ 1.0f , 1.0f }, // Top-right
		{ 1.0f , 0.0f }  // Bottom-right
	};

	for (int i = 0; i < 4; ++i)
	{
		billboardVertices[i].PositionHS = mul(float4(gsIn[0].PositionVS + basisVectors[i] * float3(gsIn[0].ParticleSize / 2.0f, 0.0f), 1.0f), g_cameraProjectionMatrix);
		billboardVertices[i].ParticleColor = gsIn[0].ParticleColor;
		billboardVertices[i].ParticleUV = texCoords[i];
		pVertexOutStream.Append(billboardVertices[i]);
	}
}

float4 PS(ParticleVertexGSOut inGSOut) : SV_Target
{
	return g_diffuseMap.Sample(g_linearSampler, inGSOut.ParticleUV);
	//return inGSOut.ParticleColor;
}