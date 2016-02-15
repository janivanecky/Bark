struct VertexShaderInput
{
	float3 pos : POSITION;
	float3 normal : NORMAL;
	uint instanceID: SV_INSTANCEID;
};

struct VertexShaderOutput
{
	float4 pos : SV_POSITION;
	float4 color: COLOR;
	float3 normal: NORMAL;
};

cbuffer SceneBuffer: register(b0)
{
	matrix projection : PROJECTION;
	matrix view: VIEW;
};

cbuffer ModelBuffer: register(b1)
{
	matrix model : MODEL;
	float4 color: COLOR;
};

cbuffer ControlBuffer: register(b10)
{
	int instancingOn: INSTANCE_CONTROL;
}

struct ModelData
{
	matrix model;
	float4 color;
};

cbuffer InstanceBuffer: register(b11)
{
	ModelData models[800]: MODEL_INSTANCED;
}

VertexShaderOutput main(VertexShaderInput input)
{
	VertexShaderOutput output;
	float4 pos = float4(input.pos, 1.0f);
	if (instancingOn)
	{
		pos = mul(models[input.instanceID].model, pos);
		output.color = models[input.instanceID].color;
	}
	else
	{
		pos = mul(model, pos);
		output.color = color;
	}
	pos = mul(view, pos);
	pos = mul(projection, pos);
	output.pos = pos;
	output.normal = input.normal;
	return output;
}
