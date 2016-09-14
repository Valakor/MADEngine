#include "Common.hlsl"

// Input structs for vertex and pixel shader
struct VS_INPUT
{
	float3 mPos : POSITION;
};

struct PS_INPUT
{
	float4 mPos : SV_POSITION;
};

// Convert clip space coordinates to view space
float4 ClipToView(float4 clip)
{
	// View space position
	float4 view = mul(clip, g_cameraInverseProjectionMatrix);
	// Perspective projection
	view = view / view.w;

	return view;
}

// Convert screen space coordinates to view space
float4 ScreenToView(float4 screen)
{
	// Convert to normalized texture coordinates
	float2 texCoord = screen.xy / g_screenDimensions;

	// Convert to clip space
	float4 clip = float4(float2(texCoord.x, 1.0f - texCoord.y) * 2.0f - 1.0f, screen.z, screen.w);

	return ClipToView(clip);
}


//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output;

	output.mPos = float4(input.mPos, 1.0f);
	return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(PS_INPUT input) : SV_Target
{
	int3 texCoord = int3(input.mPos.xy, 0);

	float2 sampleNormal = g_normalBuffer.Load(texCoord).xy;
	float4 sampleSpecular = g_specularBuffer.Load(texCoord);
	float depth = g_depthBuffer.Load(texCoord).r;

	float3 N = float3(sampleNormal.x, sampleNormal.y, sqrt(1.0f - dot(sampleNormal.xy, sampleNormal.xy)));
	float3 diffuseColor = g_diffuseBuffer.Load(texCoord).rgb;
	float3 specularColor = sampleSpecular.rgb;
	float specularPower = exp2(sampleSpecular.a * 10.5f);

	float3 posVS = ScreenToView(float4(texCoord.xy, depth, 1.0f)).xyz;

	// Do lighting!
	float3 L = normalize(-g_directionalLight.m_lightDirection);
	float3 V = normalize(-posVS);
	float3 R = normalize(reflect(-L, N));

	float I = g_directionalLight.m_lightIntensity;

	float NdotL = max(dot(N, L), 0.0f);
	float3 diffuse = NdotL * diffuseColor * g_directionalLight.m_lightColor;

	float RdotV = max(dot(R, V), 0.0f);
	float3 specular = pow(RdotV, specularPower) * specularColor;

	float3 phong = saturate(I * (diffuse + specular));
	return float4(phong, 1.0f);
}
