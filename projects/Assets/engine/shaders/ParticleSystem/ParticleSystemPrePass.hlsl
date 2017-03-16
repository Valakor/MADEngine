#include "ParticleSystemCommon.hlsl"

ParticleVertex VS(ParticleVertex vsIn)
{
	return vsIn;
}

// Stream-Out GS responsible for emitting new particles and destroying old particles.
// Operates on entire primitives (contrasting to the vertex shader, which operates on individual vertices)
[maxvertexcount(150)]
void GS(point ParticleVertex particleIn[1], inout PointStream<ParticleVertex> particleStream)
{
	// Based on the age, velocity, and game time, we will spawn/kill particles here

	// For now, just pass the particle through
	particleStream.Append(particleIn[0]);
}

// NO pixel shader