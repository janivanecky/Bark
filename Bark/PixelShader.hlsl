struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float4 color: COLOR;
	float3 normal: NORMAL;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
	float3 lightDir = normalize(float3(1,2,-3));
	float d = clamp(dot(lightDir, input.normal), 0.0f, 1.0f);
	float4 color = input.color;
	float3 ambient = color.xyz * float3(1,1,1);//float3(0.4f, 0.4f, 0.4);
	float3 diffuse = color.xyz * float3(d * 0.6f, d * 0.6f, d * 0.6f);
	float4 result = float4(clamp(ambient, 0, 1) , 1.0f);
	return float4(pow(result.xyz, 1.0 / 2.2), result.a);  
}
