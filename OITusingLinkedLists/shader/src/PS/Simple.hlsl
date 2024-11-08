TextureCube  skyboxMap : register( t0 );
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

SamplerState defaultSampler : register( s0 );

void blinn(float3 normal, float3 viewDir, out float3 diffColor, out float3 specColor, out float3 ambColor)
{
	const float specPower = 30.0;

	diffColor = float3(0, 0, 0);
	specColor = float3(0, 0, 0);
	ambColor = float3(0, 0, 0);
	for (uint i = 0; i < 2; i++)
	{
		float ndol = max(0.0, dot(lightsData[i].direction, normal));
		diffColor += lightsData[i].diffuseColor * ndol;
		
		float3 h = normalize(viewDir + lightsData[i].direction);
		specColor += lightsData[i].specularColor * pow (max(dot(normal, h), 0.0), specPower);
		
		ambColor += lightsData[i].ambientColor;
	}
}

float3 computeColorOpaque(float3 pos, float3 normal)
{
	normal = -normalize(normal);
	
	float3 viewDir = normalize(pos.xyz - float3(g_mInvView._41,g_mInvView._42,g_mInvView._43));
	float3 diffColor, specColor, ambColor;
	blinn(normal, viewDir, diffColor, specColor, ambColor);
	
	float3 reflectVec = reflect(viewDir, normal);
	float3 envColor = skyboxMap.Sample(defaultSampler, reflectVec).rgb;
	float3 diffuse = envColor * (diffColor + ambColor);
	
	return saturate(diffuse + specColor);
}

//Texture2D shadowMap : register( t0 );
//Texture2DMS<float4> map : register( t0 );

//SamplerState linearSampler : register( s0 );

ThreeTargets PS(in float4 color : TEXCOORD0)
{ 
   ThreeTargets output;

   output.target0 = float4( 0, 0, 0, 1.0 );

   output.target1 = float4( 0, 0, 0, 1.0 );

   output.target2 = color;

   return  output;
}

float4 CONST_COLOR(in float4 tex : TEXCOORD0):SV_TARGET
{ 
   float4 clip_pos = tex / tex.w;

   float2 tx = float2(clip_pos.x*0.5+0.5,clip_pos.y*-0.5+0.5);

   // float Z = map.Load( int2(tx.x*g_vScreenResolution.x, tx.y*g_vScreenResolution.y), 0 ).x;

   // if( (linearDepth(0.1, 1000, clip_pos.z)) < linearDepth(0.1, 1000, Z) )
   
   //    discard;

   return float4( 0.0, 0.0, 1.0, 1.0 );   
}

float4 REF_COLOR(
    in float3 pos : TEXCOORD0,
    in float3 normal : TEXCOORD1,
    in float2 tex_0 : TEXCOORD2,
    in float4 tex_1 : TEXCOORD3,
    in float4 position : SV_POSITION
):SV_TARGET
{ 
   return float4( computeColorOpaque(pos, normal), 0.0 );   
}
