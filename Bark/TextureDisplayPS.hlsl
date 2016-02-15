Texture2D screenTexture;
SamplerState textureSampler;

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 texCoord : TEXCOORD;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
	float4 color = screenTexture.Sample(textureSampler, input.texCoord);
	//color.r *= 0.85f;
	//color.b *= 0.9f;
	float2 dist = input.texCoord - float2(0.5f, 0.5f);
	//dist.y *= 1080.0f / 1920.0f;
	dist.y *= 600.0f / 800.0f;
	float distanceToMiddle = clamp(length(dist) * 1.25f, 0, 10);
	distanceToMiddle *= distanceToMiddle * distanceToMiddle;
	color.r -= distanceToMiddle * 1.5f;
	color.g -= distanceToMiddle * 1.5f;
	color.b -= distanceToMiddle * 1.5f;
	return color;
}