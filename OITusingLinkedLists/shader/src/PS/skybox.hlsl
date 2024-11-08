TextureCube  skyboxMap : register( t0 );
SamplerState defaultSampler : register( s0 );

float4 PS(in float3 tex : TEXCOORD0) : SV_TARGET
{
	float4 color = saturate(skyboxMap.Sample(defaultSampler, tex));

	return float4(color.xyz, 1);
}