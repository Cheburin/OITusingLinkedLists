struct ListNode
{
	uint packedColor;
	uint depthAndCoverage;
	uint next;
};
RWStructuredBuffer<ListNode> fragmentsList : register( u1 );
RWTexture2D<uint> headBuffer : register( u2 );

uint packColor(float4 color)
{
	return (uint(color.r * 255) << 24) | (uint(color.g * 255) << 16) | (uint(color.b * 255) << 8) | uint(color.a * 255);
}

float4 computeColorTransparent(in float3 pos, in float3 n, in bool frontFace)
{
	float3 normal = normalize(n) * (frontFace ? -1.0f : 1.0f);

	float3 viewDir = normalize(pos.xyz - float3(g_mInvView._41,g_mInvView._42,g_mInvView._43));
	float3 diffColor, specColor, ambColor;
	blinn(normal, viewDir, diffColor, specColor, ambColor);

	float3 reflectVec = reflect(viewDir, normal);
	float3 envColor = skyboxMap.Sample(defaultSampler, reflectVec).rgb;
	float alpha = clamp(1.0f - dot(viewDir, normal), 0.3f, 1.0f);

	float3 diffuse = envColor * (diffColor + ambColor);

	return float4(saturate(diffuse + specColor), alpha);    
}

[earlydepthstencil]
float4 LIST_CREATION(
    in float3 pos : TEXCOORD0,
    in float3 normal : TEXCOORD1,
    in float2 tex_0 : TEXCOORD2,
    in float4 tex_1 : TEXCOORD3,
    in float4 position : SV_POSITION,
    uint coverage : SV_COVERAGE,
    bool frontFace : SV_IsFrontFace
) : SV_TARGET
{ 
	uint newHeadBufferValue = fragmentsList.IncrementCounter();
	if (newHeadBufferValue == 0xffffffff) { return float4(0, 0, 0, 0); }
	
	uint2 upos = uint2(position.xy);
	uint previosHeadBufferValue;
	InterlockedExchange(headBuffer[upos], newHeadBufferValue, previosHeadBufferValue);

	ListNode node;

	float4 color = computeColorTransparent(pos, normal, frontFace);

	node.packedColor = packColor(float4(color.rgb, color.a));
	node.depthAndCoverage = f32tof16(tex_1.z) | (coverage << 16);
	node.next = previosHeadBufferValue;
	fragmentsList[newHeadBufferValue] = node;

	return float4(0, 0, 0, 0);
}