struct VertexShaderInput
{
	float3 pos : POSITION;
	float2 texCoord : TEXCOORD;
};

struct VertexShaderOutput
{
	float4 pos : SV_POSITION;
	float2 texCoord : TEXCOORD;
};

VertexShaderOutput main(VertexShaderInput input)
{
	VertexShaderOutput output;
	output.pos = float4(input.pos, 1.0f);
	output.texCoord = input.texCoord;
	return output;
}