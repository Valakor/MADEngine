#pragma pack_matrix(row_major)

struct Particle
{
	float3 InitialPosVS  : POSITION;
	float3 InitialVelVS  : VELOCITY;
	float4 ParticleColor : COLOR;
	float2 ParticleSize  : SIZE;
	float  Age			 : AGE;
};

float3x3 CreateLookAt(float3 inEye, float3 inTarget, float3 inUpHint)
{
	float3x3 outputLookAt;

	float3 forward;
	float3 up;
	float3 right;

	// Singularity where the eye is exactly at the target look at position
	/*if (inEye == inTarget)
	{
		return outputLookAt;
	}*/

	forward = normalize(inTarget - inEye); // Forward
	
	/*if (normalize(outputLookAt[2]) == inUpHint)
	{
		// Singularity where the eye is looking at the up hint direction, which would result in a cross product of 0
		inUpHint = { 0.0f, 0.0f, 1.0f };
	}*/

	// Right
	right = cross(normalize(inUpHint), forward);

	// Up
	up = cross(forward, right);

	outputLookAt[0] = right;
	outputLookAt[1] = up;
	outputLookAt[2] = forward;

	return outputLookAt;
}