#pragma once
#include <d3d11.h>
#include "PlatformFunctions.h"
#include "Core\Math.h"

#define SCENE_SLOT 0
#define MODEL_SLOT 1
#define CONTROL_SLOT 10
#define INSTANCE_SLOT 11
#define COLOR_SLOT 12
#define SPRITE_SLOT 13


struct Renderer
{
	ID3D11Device *device;
	ID3D11DeviceContext *context;
	Vector2 screenSize;
	ID3D11SamplerState *texSampler;
};

struct RenderTarget
{
	ID3D11RenderTargetView *renderTarget;
	ID3D11DepthStencilView *depthStencil;
	ID3D11Texture2D *resource;
	uint32 width;
	uint32 height;
};

struct ConstantBuffer
{
	ID3D11Buffer *buffer;
};

struct Pipeline
{
	ID3D11VertexShader *vertexShader;
	ID3D11PixelShader *pixelShader;
	ID3D11InputLayout *inputLayout;
};

enum InputType
{
	FLOAT1 = 0,
	FLOAT2 = 1,
	FLOAT3 = 2,
	FLOAT4 = 3,
};

struct VertexInputLayout
{
	char **names;
	InputType *components;
	uint32 inputCount;
	char **instanceNames;
	InputType *instanceComponents;
	uint32 instanceCount;
};

struct Model
{
	ID3D11Buffer *vertexBuffer;
	ID3D11Buffer *indexBuffer;
	uint32 vertexCount;
	uint32 vertexStride;
	uint32 indexCount;
};

enum BufferType
{
	NORMAL,
	DYNAMIC
};

enum PrimitiveType
{
	TRIANGLES,
	LINES
};


struct Texture
{
	ID3D11ShaderResourceView *shaderResourceView;
	uint32 width;
	uint32 height;
};

struct TextureDesc
{
	uint32 width;
	uint32 height;
	uint32 mipCount;
	void *data;
};

struct PerModelData
{
	Matrix4x4 modelMatrix;
	bool instancingOn = false;
	uint32 instanceBaseOffset;
};


namespace Graphics
{
	void InitPipeline(Renderer *renderer, Pipeline *pipeline, VertexInputLayout inputLayout,
					FileData *vertexShaderFile, FileData *pixelShaderFile);
	void InitModel(Renderer *renderer, Model *model, float *vertices, uint32 vertexCount, uint32 vertexStride,
				   uint16 *indices, int32 indexCount, BufferType type = NORMAL);
	void InitConstantBuffer(Renderer *renderer, ConstantBuffer *buffer, uint32 dataSize);
	void ClearTarget(Renderer *renderer, RenderTarget *renderTarget);
	void BindTarget(Renderer *renderer, RenderTarget *renderTarget);
	void BindTargetDepthless(Renderer *renderer, RenderTarget *renderTarget);
	void BindPipeline(Renderer *renderer, Pipeline *pipeline);
	void BindConstantBuffer(Renderer *renderer, ConstantBuffer *buffer, void *data, uint32 slot);
	void UpdateModelData(Renderer *renderer, Model *model, void *data, uint32 size, uint32 vertexCount = 0);
	void RenderModel(Renderer *renderer, Model *model, PrimitiveType type = TRIANGLES);
	void RenderModelInstanced(Renderer *renderer, Model *model, uint32 instanceCount, 
							  PrimitiveType type = TRIANGLES);

	void Release(Pipeline *pipeline);
	void Release(Model *model);
	void Release(ConstantBuffer *buffer);

	void CreateWhiteTexture(Renderer *renderer, Texture *texture);
	void InitTexture(Renderer *renderer, Texture *texture, TextureDesc textureDesc);
	void BindTexture(Renderer *renderer, Texture *texture);

}

