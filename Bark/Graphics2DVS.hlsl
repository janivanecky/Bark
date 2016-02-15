cbuffer PerSceneBuffer: register(b0)
{
	matrix projection;
	matrix view;
};

cbuffer PerModelBuffer: register(b1)
{
	matrix model;
	bool instancingOn;
	uint instanceBaseOffset;
};

cbuffer ColorBuffer: register(b12)
{
	float4 col: COLOR;
};

cbuffer SpriteBuffer: register(b13)
{
	float4 sourceRect: RECT;
};

struct VertexShaderInput
{
	float3 pos : POSITION;
	float2 texCoord : TEXCOORD;
};

struct VertexShaderOutput
{
	float4 pos : SV_POSITION;
	float2 texCoord : TEXCOORD;
	float4 col : COLOR;
};

VertexShaderOutput main(VertexShaderInput input)
{
	VertexShaderOutput output;

	output.pos = float4(input.pos, 1.0f);
	output.pos = mul(model, output.pos);
	output.pos = mul(projection, output.pos);

	output.col = col;

	float2 texCoord = input.texCoord;
	texCoord.x = texCoord.x * sourceRect.z + sourceRect.x;
	texCoord.y = texCoord.y * sourceRect.w + sourceRect.y;
	output.texCoord = texCoord;

	return output;
}