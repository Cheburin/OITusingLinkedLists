//--------------------------------------------------------------------------------------
// Defines
//--------------------------------------------------------------------------------------                  
#define ADD_SPECULAR 0

//--------------------------------------------------------------------------------------
// Structures
//--------------------------------------------------------------------------------------

struct PosNormalTex2d
{
	float3 pos : SV_Position;
	float3 normal   : NORMAL;
	float2 tex      : TEXCOORD0;
};

struct PosNormalTangetColorTex2d
{
	float3 pos      : SV_Position;
	float3 normal   : NORMAL;
	float4 tangent  : TANGENT;
	float4 color    : COLOR0;
	float2 tex      : TEXCOORD0;
};

struct ExpandPosNormalTangetColorTex2d
{
	float3 normal   : NORMAL;
	float4 tangent  : TANGENT;
	float4 color    : COLOR0;
	float2 tex      : TEXCOORD0;
	float4 pos      : SV_POSITION;
};

struct ExpandPosNormalTex2d
{
	float3 normal   : NORMAL;
	float2 tex      : TEXCOORD0;
	float4 pos      : SV_POSITION;
};

struct ExpandPos
{
	float4 pos      : SV_POSITION;
};

struct ClipPosTex2d
{
	float2 tex            : TEXCOORD0;
	float4 clip_pos       : SV_POSITION; // Output position
};

struct ClipPosTex4d
{
	float4 tex            : TEXCOORD0;
	float4 clip_pos       : SV_POSITION; // Output position
};

struct ClipPosTex3d
{
	float3 tex            : TEXCOORD0;
	float4 clip_pos       : SV_POSITION; // Output position
};

struct ClipPosPosNormalTex2d
{
	float3 pos            : TEXCOORD0;
	float3 normal         : TEXCOORD1;   // Normal vector in world space
	float2 tex            : TEXCOORD2;
	float4 clip_pos       : SV_POSITION; // Output position
};

struct ClipPosPosNormalTex2dTex4d
{
	float3 pos            : TEXCOORD0;
	float3 normal         : TEXCOORD1;   // Normal vector in world space
	float2 tex_0          : TEXCOORD2;
	float4 tex_1          : TEXCOORD3;
	float4 clip_pos       : SV_POSITION; // Output position
};

struct ClipPosPosNormalTangentBitangentTex2d
{
	float3 pos            : TEXCOORD0;
	float3 normal         : TEXCOORD1;
	float3 tangent        : TEXCOORD2;
	float3 bitangent      : TEXCOORD3;
	float2 tex            : TEXCOORD4;
	float4 clip_pos       : SV_POSITION; // Output position
};

struct ClipPosColor
{
	float4 color          : TEXCOORD0;
	float4 clip_pos       : SV_POSITION; // Output position
};

struct ThreeTargets
{
	float4 target0: SV_Target0;

	float4 target1: SV_Target1;

	float4 target2: SV_Target2;
};

//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------
cbuffer cbMain : register(b0)
{
	matrix    g_mWorld;                         // World matrix
	matrix    g_mView;                          // View matrix
	matrix    g_mProjection;                    // Projection matrix
	matrix    g_mWorldViewProjection;           // WVP matrix
	matrix    g_mWorldView;                     // WV matrix
	matrix    g_mInvView;                       // Inverse of view matrix

	matrix    g_mObject1View;                // VP matrix
	matrix    g_mObject1WorldView;                       // Inverse of view matrix
	matrix    g_mObject1WorldViewProjection;                       // Inverse of view matrix

	matrix    g_mObject2View;                // VP matrix
	matrix    g_mObject2WorldView;                       // Inverse of view matrix
	matrix    g_mObject2WorldViewProjection;                       // Inverse of view matrix

	float4    g_vFrustumNearFar;              // Screen resolution
	float4    g_vFrustumParams;              // Screen resolution
	float4    g_viewLightPos;                   //
};

float linearDepth(float near, float far, float depth){
	return near * far / (far - depth * (far - near));
}

ClipPosPosNormalTex2dTex4d REF_MODEL(in PosNormalTex2d i)
{
	ClipPosPosNormalTex2dTex4d output;

	output.pos = mul(float4(i.pos, 1.0), g_mWorld).xyz;

	output.normal = mul(float4(i.normal, 0.0), g_mWorld).xyz;

	output.tex_0 = i.tex;

	float4 clip_pos = mul(float4(i.pos, 1.0), g_mWorldViewProjection);

		output.clip_pos = output.tex_1 = clip_pos;

	return output;
}

TextureCube  skyboxMap : register(t0);
struct LightData
{
	float3 position;
	uint lightType;
	float3 direction;
	float falloff;
	float3 diffuseColor;
	float angle;
	float3 ambientColor;
	uint dummy;
	float3 specularColor;
	uint dummy2;
};
StructuredBuffer<LightData> lightsData : register(t1);

SamplerState defaultSampler : register(s0);

void blinn(float3 normal, float3 viewDir, out float3 diffColor, out float3 specColor, out float3 ambColor)
{
	const float specPower = 30.0;

	diffColor = float3(0, 0, 0);
	specColor = float3(0, 0, 0);
	ambColor = float3(0, 0, 0);
	for (uint i = 0; i < 1; i++)
	{
		float ndol = max(0.0, dot(lightsData[i].direction, normal));
		//diffColor += lightsData[i].diffuseColor * ndol;

		float3 h = normalize(viewDir + lightsData[i].direction);
			specColor += lightsData[i].specularColor * pow(max(dot(normal, h), 0.0), specPower);

		ambColor += lightsData[i].ambientColor;
	}
}

float3 computeColorOpaque(float3 pos, float3 normal)
{
	normal = -normalize(normal);

	float3 viewDir = normalize(pos.xyz - float3(g_mInvView._41, g_mInvView._42, g_mInvView._43));
		float3 diffColor, specColor, ambColor;
	blinn(normal, viewDir, diffColor, specColor, ambColor);

	//float3 reflectVec = reflect(viewDir, normal);
	float3 envColor = float3(1, 1, 1);//skyboxMap.Sample(defaultSampler, reflectVec).rgb;
		float3 diffuse = envColor * (diffColor + ambColor);

		return saturate(diffuse + specColor);
}


float4 REF_COLOR(in float3 pos : TEXCOORD0, in float3 normal : TEXCOORD1, in float2 tex_0 : TEXCOORD2, in float4 tex_1 : TEXCOORD3) :SV_TARGET
{
	return float4(computeColorOpaque(pos, normal), 1.0);
}
