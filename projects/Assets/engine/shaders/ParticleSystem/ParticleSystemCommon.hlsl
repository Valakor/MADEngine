#pragma pack_matrix(row_major)

cbuffer CBParticlePerFrameConstants : cbuffer(b0)
{
	float4x4 g_projectionMatrix;
	float g_gameTime;
	float g_frameTime;
}

cbuffer CBParticleFixedConstants : cbuffer(b1)
{
	float3 g_gravitationalAccel = { 0.0f, 9.8f, 0.0f };
}

struct Particle
{
	float3 InitialPosVS  : POSITION;
	float3 InitialVelVS  : VELOCITY;
	float4 ParticleColor : COLOR;
	float2 ParticleSize  : SIZE;
	//uint   Type			 : TYPE;
};

float3x3 CreateLookAt(float3 inEye, float3 inTarget, float3 inUpHint)
{
	float3x3 outputLookAt;

	// Singularity where the eye is exactly at the target look at position
	if (inEye == inTarget)
	{
		return outputLookAt;
	}


	outputLookAt[2] = normalize(inTarget - inEye); // Forward
	
	// Left
	if (normalize(outputLookAt[0]) == inUpHint)
	{
		// Singularity where the eye is looking at the up hint direction, which would result in a cross product of 0
		inUpHint = { 0.0f, 0.0f, 1.0f };
	}

	outputLook[0] = cross(normalize(inUpHint), outputLookAt[2]);

	// Up
	outputLook[1] = cross(outputLookAt[2], outputLook[0]);
}