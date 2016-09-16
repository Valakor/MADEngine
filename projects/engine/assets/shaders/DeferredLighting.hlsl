#include "Common.hlsl"

// Input structs for vertex and pixel shader
struct PS_INPUT
{
	float4 mPos : SV_POSITION;
	float2 mTexCoord : TEXCOORD0;
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

// Decodes a two-component half-precision pair in the range [0-1]
// into a 3-component view space normal.
// See: http://aras-p.info/texts/CompactNormalStorage.html
float3 DecodeNormal(half2 inEnc)
{
	half2 fenc = inEnc * 4.0 - 2.0;
	half f = dot(fenc, fenc);
	half g = sqrt(1.0 - f / 4.0);
	half3 n;
	n.xy = fenc * g;
	n.z = 1.0 - f / 2.0;
	return n;
}


//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS(uint id : SV_VertexID)
{
	PS_INPUT output;

	output.mTexCoord = float2(id & 1, id >> 1);
	output.mPos = float4((output.mTexCoord.x - 0.5f) * 2.0f, -(output.mTexCoord.y - 0.5f) * 2.0f, 0.0f, 1.0f);
	return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(PS_INPUT input) : SV_Target
{
	int3 texCoord = int3(input.mPos.xy, 0);

	half2 sampleNormal = g_normalBuffer.Load(texCoord).xy;
	float4 sampleSpecular = g_specularBuffer.Load(texCoord);
	float depth = g_depthBuffer.Load(texCoord).r;

	float3 N = DecodeNormal(sampleNormal);
	float3 diffuseColor = g_diffuseBuffer.Load(texCoord).rgb;
	float3 specularColor = sampleSpecular.rgb;
	float specularPower = exp2(sampleSpecular.a * 10.5f);

	float3 posVS = ScreenToView(float4(texCoord.xy, depth, 1.0f)).xyz;

	// Do lighting!
	float3 L = normalize(-g_directionalLight.m_lightDirection);
	float3 V = normalize(-posVS);
	float3 R = normalize(reflect(-L, N));

	float I = g_directionalLight.m_lightIntensity;

	//float NdotL = max(dot(N, L), 0.0f); // Normal Phong
	float NdotL = pow(dot(N, L) * 0.5f + 0.5f, 2.0f); // Valve half-Lambert
	float3 diffuse = NdotL * diffuseColor * g_directionalLight.m_lightColor;

	float RdotV = max(dot(R, V), 0.0f);
	float3 specular = pow(RdotV, specularPower) * specularColor;

	float3 phong = saturate(I * (diffuse + specular));
	return float4(phong, 1.0f);
}
