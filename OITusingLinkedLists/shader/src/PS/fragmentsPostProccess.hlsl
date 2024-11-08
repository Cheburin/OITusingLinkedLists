struct ListNode
{
	 uint packedColor;
	 uint depthAndCoverage;
	 uint next;
};
RWStructuredBuffer<ListNode> fragmentsList : register( u1 );
RWTexture2D<uint> headBuffer : register( u2 );

static const int MAX_FRAGMENTS = 16;

struct NodeData
{
	uint packedColor;
	float depth;
};

float4 unpackColor(uint color)
{
	float4 output;
	output.r = float((color >> 24) & 0x000000ff) / 255.0f;
	output.g = float((color >> 16) & 0x000000ff) / 255.0f;
	output.b = float((color >> 8) & 0x000000ff) / 255.0f;
	output.a = float(color & 0x000000ff) / 255.0f;
	return output; //saturate
}

void insertionSortMSAA(uint startIndex, uint sampleIndex, inout NodeData sortedFragments[MAX_FRAGMENTS], out int counter)
{
	counter = 0;
	uint index = startIndex;
	for (int i = 0; i < MAX_FRAGMENTS; i++)
	{
		if (index != 0xffffffff)
		{
			uint coverage = (fragmentsList[index].depthAndCoverage >> 16);
			if (coverage & (1 << sampleIndex))
			{
				sortedFragments[counter].packedColor = fragmentsList[index].packedColor;
				sortedFragments[counter].depth = f16tof32(fragmentsList[index].depthAndCoverage);
				counter++;
			}
			index = fragmentsList[index].next;
		}
	}

	for (int k = 1; k < MAX_FRAGMENTS; k++)
	{
		int j = k;
		NodeData t = sortedFragments[k];

		while (sortedFragments[j - 1].depth < t.depth)
		{
			sortedFragments[j] = sortedFragments[j - 1];
			j--;
			if (j <= 0) { break; }
		}

		if (j != k) { sortedFragments[j] = t; }
	}   
}

float4 PS(
   in float2 tex : TEXCOORD0,
   in float4 position : SV_POSITION,
   uint sampleIndex : SV_SAMPLEINDEX
) : SV_TARGET
{
	uint2 upos = uint2(position.xy);
	uint index = headBuffer[upos];
	clip(index == 0xffffffff ? -1 : 1);
	
	float3 color = float3(0, 0, 0);
	float alpha = 1;
	
	NodeData sortedFragments[MAX_FRAGMENTS];
	[unroll]
	for (int j = 0; j < MAX_FRAGMENTS; j++)
	{
		sortedFragments[j] = (NodeData)0;
	}

	int counter;
	insertionSortMSAA(index, sampleIndex, sortedFragments, counter);

	// resolve multisampling
	int resolveBuffer[MAX_FRAGMENTS];
	float4 colors[MAX_FRAGMENTS];

   for(int l = 0; l < MAX_FRAGMENTS; l++){
      colors[l] = float4(0, 0, 0, 0);
      resolveBuffer[l] = 0;
   }   

	int resolveIndex = -1;
	float prevdepth = -1.0f;
	[unroll(MAX_FRAGMENTS)]
	for (int i = 0; i < counter; i++)
	{
		if (sortedFragments[i].depth != prevdepth)
		{
			resolveIndex = -1;
			resolveBuffer[i] = 1;
			colors[i] = unpackColor(sortedFragments[i].packedColor);
		}
		else
		{
			if (resolveIndex < 0) { resolveIndex = i - 1; }

			colors[resolveIndex] += unpackColor(sortedFragments[i].packedColor);
			resolveBuffer[resolveIndex]++;

			resolveBuffer[i] = 0;
		}
		prevdepth = sortedFragments[i].depth;
	}

	// gather
	[unroll(MAX_FRAGMENTS)]
	for (int k = 0; k < counter; k++)
	{
		[branch]
		if (resolveBuffer[k] != 0)
		{
			float4 c = colors[k] / float(resolveBuffer[k]);
			alpha *= (1.0 - c.a);
			color = lerp(color, c.rgb, c.a);
		}
	}

   return float4(color, alpha);   
}