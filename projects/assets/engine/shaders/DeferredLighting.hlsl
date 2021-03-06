#include "Common.hlsl"

//=>:(Usage, VS, vs_5_0)
//=>:(Usage, PS, ps_5_0)
//=>:(Permute, POINT_LIGHT)
//=>:(Permute, DIRECTIONAL_LIGHT)

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

// Decodes a specular power in the range [0-1] to the
// range [1-1448.15]. Provides best precision for values in the
// range [1-256].
// See: http://www.3dgep.com/forward-plus/#Specular_Buffer
float DecodeSpecPower(float inEncodedSpecularPower)
{
	return exp2(inEncodedSpecularPower * 10.5);
}

// Calculate L based upon what type of light we're using
float3 CalculateL(float3 positionVS)
{
#ifdef POINT_LIGHT
	return normalize(g_pointLight.m_lightPosition - positionVS); // We transform this to VS on the CPU
#else
	return normalize(-g_directionalLight.m_lightDirection.xyz); // We transform this to VS on the CPU
#endif
}

// Calculate light attenuation based upon what type of light we're using
float CalculateShadowFactor(float3 positionVS)
{
#ifdef POINT_LIGHT
	float4 positionWS = mul(float4(positionVS, 1.0), g_cameraInverseViewMatrix);
	float4 lightPositionWS = mul(float4(g_pointLight.m_lightPosition, 1.0), g_cameraInverseViewMatrix);
	float3 sampleVec = positionWS.xyz - lightPositionWS.xyz;

	sampleVec.z = -sampleVec.z; // Reverse the z since our coordinate system is negative z forward

	float calculatedDepth = 0.0;

	// Goal: Retrieve the depth of the pixel with correct perspective
	// Potential Solution: Go through each VP matrix and see which once produces a value in the correct range, the z value will be the depth
	for (int i = 0;i < 6; ++i)
	{
		float4 positionLS = mul(positionWS, g_pointLightVPMatrices[i]);

		positionLS.xyz /= positionLS.w;

		// Check the coordinates of the position to see if its in the correct NDC range
		if (positionLS.x >= -1.0 && positionLS.x <= 1.0 && positionLS.y >= -1.0 && positionLS.y <= 1.0 && positionLS.z >= 0 && positionLS.z <= 1.0)
		{
			calculatedDepth = positionLS.z;
			break;
		}
	}
	
	return g_cubeMap.SampleCmpLevelZero(g_shadowMapSampler, sampleVec, calculatedDepth).r;
#elif DIRECTIONAL_LIGHT
	float4 positionWS = mul(float4(positionVS, 1.0), g_cameraInverseViewMatrix);
	float4 positionLS = mul(positionWS, g_directionalLight.m_viewProjectionMatrix);
	positionLS.xyz /= positionLS.w;

	if (positionLS.z > 1.0)
	{
		return 1.0;
	}

	positionLS.x = positionLS.x * 0.5 + 0.5;
	positionLS.y = 0.5 - positionLS.y * 0.5;

	//float w, h;
	//g_shadowMap.GetDimensions(w, h);
	const float dx = 1.0 / 4096;
	const float2 offsets[5] =
	{
		                   float2(0.0f,  -dx),
		float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
		                   float2(0.0f,   dx)
	};

	float shadowFactor = 0.0;

	[unroll]
	for (int i = 0; i < 5; ++i)
	{
		shadowFactor += g_diffuseMap.SampleCmpLevelZero(g_shadowMapSampler, positionLS.xy + offsets[i], positionLS.z).r;
	}

	return shadowFactor / 5.0;
#else
	return 1.0;
#endif
}

// Calculate light attenuation based upon what type of light we're using
void GetLightIrradianceProperties(float3 positionVS, out float attenuation, out float3 lightColor, out float lightIntensity)
{
	float shadowFactor = CalculateShadowFactor(positionVS);

#ifdef POINT_LIGHT
	float d = distance(positionVS, g_pointLight.m_lightPosition);
	d = max(d - g_pointLight.m_lightInnerRadius, 0.0);
	attenuation = clamp(1.0 - d / (g_pointLight.m_lightOuterRadius - g_pointLight.m_lightInnerRadius), 0.0, 1.0);
	attenuation *= attenuation;

	lightColor = g_pointLight.m_lightColor;
	lightIntensity = g_pointLight.m_lightIntensity;
#elif DIRECTIONAL_LIGHT
	attenuation = 1.0;
	lightColor = g_directionalLight.m_lightColor;
	lightIntensity = g_directionalLight.m_lightIntensity;
#else
	attenuation = 1.0;
	lightColor = float4(1.0, 1.0, 1.0, 1.0);
	lightIntensity = 1.0;
#endif

	attenuation *= shadowFactor;
}


//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output;
	output.mPos = float4(input.mPos, 1.0);
	return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(PS_INPUT input) : SV_Target
{
	// { screenX, screenY, 0 }
	int3 texCoord = int3(input.mPos.xy, 0);

	// Sample all of our buffers
	float4 sampleDiffuse  = g_diffuseBuffer.Load(texCoord);
	float4 sampleSpecular = g_specularBuffer.Load(texCoord);
	half2  sampleNormal   = g_normalBuffer.Load(texCoord).xy;
	float  sampleDepth    = g_depthBuffer.Load(texCoord).r;

	// Rebuild our material properties
	float3 materialDiffuseColor  = sampleDiffuse.rgb;
	float3 materialSpecularColor = sampleSpecular.rgb;
	float  materialSpecularPower = DecodeSpecPower(sampleSpecular.a);
	float  depth                 = sampleDepth;

	// Calculate our lighting information
	float3 positionVS = ScreenToView(float4(texCoord.xy, depth, 1.0)).xyz;
	float3 N = DecodeNormal(sampleNormal);
	float3 L = CalculateL(positionVS);
	float3 V = normalize(-positionVS);
	float3 H = normalize(L + V);

	// Directional Phong shading
	float3 phong = float3(0.0f, 0.0, 0.0);

	float NdotL = dot(N, L); // Normal Phong
	//float NdotL = pow(dot(N, L) * 0.5 + 0.5, 2.0); // Valve half-Lambert scales to [0, 1]
	if (NdotL > 0)
	{
		float3 lightColor;
		float  lightIntensity;
		float  attenuation;
		GetLightIrradianceProperties(positionVS, attenuation, lightColor, lightIntensity);

		// Calculate diffuse term
		float3 diffuse = materialDiffuseColor * NdotL;

		// Calculate specular term
		float NdotH = max(0.0, dot(N, H)); // Blinn-Phong
		float3 specular = materialSpecularColor * pow(NdotH, materialSpecularPower);

		phong = lightIntensity * lightColor * attenuation * (diffuse + specular);
	}

	return saturate(float4(phong, 1.0f));
}
