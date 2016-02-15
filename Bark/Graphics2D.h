#pragma once
#include "Graphics.h"
#include "Core\Math.h"

struct GraphicsModel;
struct Program;
struct Texture;

typedef Vector4 Color;

struct SpriteData
{
	Matrix4x4 modelMatrix;
	Rect sourceRect;
	Vector4 color;
	Texture *texture;
};

struct TextData
{
	Matrix4x4 modelMatrix;
	Rect sourceRect;
	Vector4 color;
};

struct Graphics2DContext
{
	Renderer *renderer;
	Model *screenQuad;
	Pipeline *program;
	Pipeline *textProgram;
	ConstantBuffer perModelbuffer;
	ConstantBuffer colorBuffer;
	ConstantBuffer spriteBuffer;
	Texture whiteTexture;
	Texture fontTexture[2];
	SpriteData *layers[2];
	uint32 spritePointers[2];

	TextData *layersText[2];
	uint32 textPointers[2];

	uint32 font;
};

namespace Graphics2D
{
	void Init(Graphics2DContext *graphics2DContext, Renderer *renderer, 
			  Model *screenQuad, Pipeline *program, Pipeline *textProgram);
	void Draw(Graphics2DContext *graphics2DContext, Rect rect, Vector4 color, Texture *texture, 
			  Rect sourceRect, uint32 layer = 0);
	void Draw(Graphics2DContext *graphics2DContext, Rect rect, Vector4 color, Texture *texture, 
			  uint32 layer = 0);
	void Draw(Graphics2DContext *graphics2DContext, Rect rect, Vector4 color, 
			  uint32 layer = 0);
	void DrawText(Graphics2DContext *graphics2DContext, Vector2 position, Vector2 origin, char *text, 
				  float textHeight, Vector4 color, uint32 font, uint32 layer = 0);
	void Release(Graphics2DContext *graphics2DContext);
	void ClearRender(Graphics2DContext *graphics2DContext);
	void CommitRender(Graphics2DContext *graphics2DContext);
}