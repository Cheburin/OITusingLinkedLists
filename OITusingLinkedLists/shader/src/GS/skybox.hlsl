static const float4 cubeVerts[8] = 
{
	float4(-0.5, -0.5, -0.5, 1),// LB  0
	float4(-0.5, 0.5, -0.5, 1), // LT  1
	float4(0.5, -0.5, -0.5, 1), // RB  2
	float4(0.5, 0.5, -0.5, 1),  // RT  3
	float4(-0.5, -0.5, 0.5, 1), // LB  4
	float4(-0.5, 0.5, 0.5, 1),  // LT  5
	float4(0.5, -0.5, 0.5, 1),  // RB  6
	float4(0.5, 0.5, 0.5, 1)    // RT  7
};

static const int cubeIndices[24] =
{
	0, 1, 2, 3, // front
	7, 6, 3, 2, // right
	7, 5, 6, 4, // back
	4, 0, 6, 2, // bottom
	1, 0, 5, 4, // left
	3, 1, 7, 5  // top
};

[maxvertexcount(36)]
void GS(point ExpandPos pnt[1], uint primID : SV_PrimitiveID,  inout TriangleStream<ClipPosTex3d> triStream )
{
	ClipPosTex3d v[8];
	[unroll]
	for (int j = 0; j < 8; j++)
	{
		v[j].clip_pos = mul(cubeVerts[j], g_mWorldViewProjection);
		v[j].tex = cubeVerts[j].xyz;
	}
	
	[unroll]
	for (int i = 0; i < 6; i++)
	{
		triStream.Append(v[cubeIndices[i * 4 + 1]]);
		triStream.Append(v[cubeIndices[i * 4 + 2]]);
		triStream.Append(v[cubeIndices[i * 4]]);
		triStream.RestartStrip();
		
		triStream.Append(v[cubeIndices[i * 4 + 3]]);
		triStream.Append(v[cubeIndices[i * 4 + 2]]);
		triStream.Append(v[cubeIndices[i * 4 + 1]]);
		triStream.RestartStrip();
	}
}