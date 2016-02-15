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
	//if (textureColor.a < 1.0f)
	//{
	//	textureColor.a = 1.0f;
	//}
	return input.col * textureColor;
}