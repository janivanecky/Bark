Texture2D tex;
SamplerState textureSampler;

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 texCoord : TEXCOORD;
	float4 col : COLOR;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
	float4 textureColor = tex.Sample(textureSampler, input.texCoord);
	return float4(input.col.xyz, input.col.a * textureColor.a);
}