#include "Graphics2D.h"
#include "Graphics.h"

void Graphics2D::Init(Graphics2DContext *graphics2DContext, Renderer *renderer, Model *screenQuad, 
					  Pipeline *program, Pipeline*textProgram)
{
	Graphics::InitConstantBuffer(renderer, &graphics2DContext->perModelbuffer, sizeof(PerModelData));
	Graphics::InitConstantBuffer(renderer, &graphics2DContext->colorBuffer, sizeof(Vector4));
	Graphics::InitConstantBuffer(renderer, &graphics2DContext->spriteBuffer, sizeof(Vector4));
	graphics2DContext->screenQuad = screenQuad;
	graphics2DContext->program = program;
	graphics2DContext->textProgram = textProgram;
	graphics2DContext->renderer = renderer;
	Graphics::CreateWhiteTexture(renderer, &graphics2DContext->whiteTexture);

	graphics2DContext->layers[0] = new SpriteData[500];
	graphics2DContext->layers[1] = new SpriteData[500];
	graphics2DContext->spritePointers[0] = 0;
	graphics2DContext->spritePointers[1] = 0;

	graphics2DContext->layersText[0] = new TextData[500];
	graphics2DContext->layersText[1] = new TextData[500];
	graphics2DContext->textPointers[0] = 0;
	graphics2DContext->textPointers[1] = 0;

	graphics2DContext->font = 0;
}

void Graphics2D::Draw(Graphics2DContext *graphics2DContext, Rect rect, Vector4 color, Texture *texture, 
					  Rect sourceRect, uint32 layer)
{
	
	Renderer *renderer = graphics2DContext->renderer;
	Matrix4x4 modelMatrix = Math::GetTranslation(rect.left + rect.width / 2.0f, 
												 renderer->screenSize.y - rect.top - rect.height / 2.0f, 0) *
						   Math::GetScale(rect.width / 2.0f, rect.height / 2.0f, 1);
	SpriteData newSpriteData = {};
	newSpriteData.sourceRect = sourceRect;
	newSpriteData.modelMatrix = modelMatrix;
	newSpriteData.texture = texture;
	newSpriteData.color = color;

	graphics2DContext->layers[layer][graphics2DContext->spritePointers[layer]++] = newSpriteData;

}

void Graphics2D::Draw(Graphics2DContext *graphics2DContext, Rect rect, Vector4 color, 
					  Texture *texture, uint32 layer)
{
	Graphics2D::Draw(graphics2DContext, rect, color, texture, Rect(0,0,1,1));
}

void Graphics2D::Draw(Graphics2DContext *graphics2DContext, Rect rect, Vector4 color, uint32 layer)
{
	Graphics2D::Draw(graphics2DContext, rect, color, &graphics2DContext->whiteTexture, Rect(0,0,1,1), layer);
}

float textWidths[] = 
{
	0.5f, 0.25f, 0.25f, 0.5f, 0.5f, 0.75f, 0.75f, 0.25f, 0.25f, 0.25f,
	0.25f, 0.75f, 0.25f, 0.25f, 0.25f, 0.5f, 0.5f, 0.25f, 0.5f, 0.5f, 
	0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.25f, 0.25f, 0.5f, 0.75f,
	0.5f, 0.5f, 1.0f, 0.65f, 0.6f, 0.75f, 0.7f, 0.55f, 0.5f, 0.75f,
	0.6f, 0.25f, 0.5f, 0.6f, 0.5f, 0.75f, 0.6f, 0.75f, 0.5f, 0.75f,
	0.5f, 0.5f, 0.5f, 0.65f, 0.6f, 0.9f, 0.6f, 0.6f, 0.6f, 0.25f,
	0.5f, 0.25f, 0.4f, 0.5f, 0.25f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f,
	0.25f, 0.5f, 0.5f, 0.25f, 0.25f, 0.5f, 0.25f, 0.75f, 0.5f, 0.5f,
	0.5f, 0.5f, 0.25f, 0.5f, 0.25f, 0.5f, 0.5f, 0.75f, 0.5f, 0.5f,
	0.5f, 0.25f, 0.25f, 0.25f, 0.5f,
};

void Graphics2D::DrawText(Graphics2DContext *context, Vector2 position, Vector2 origin, char *text, 
						  float textHeight, Vector4 color, uint32 font, uint32 layer)
{
	Renderer *renderer = context->renderer;
	Vector2 currentPosition = position;
	float totalWidth = 0;
	char *widthTest = text;
	while (*widthTest)
	{
		uint32 index = *widthTest++ - 32;
		float relWidth = 0.1f * textWidths[index];
		float relHeight = 0.1f;
		float width = textHeight * relWidth / relHeight;
		totalWidth += width;
	}
	currentPosition.x -= origin.x * totalWidth;
	currentPosition.y -= origin.y * textHeight;

	while (*text)
	{
		uint32 index = *text++ - 32;
		uint32 x = index % 10;
		uint32 y = index / 10;
		float gridWidth = 0.1f;
		float relWidth = 0.1f * textWidths[index];
		float relHeight = 0.1f;
		float height = textHeight;
		float width = textHeight * relWidth / relHeight;
		Rect sourceRect;
		sourceRect.left = x * 0.1f + (gridWidth - relWidth) / 2.0f;
		sourceRect.top = y * 0.1f;
		sourceRect.width = relWidth;
		sourceRect.height = relHeight;

		Rect rect(currentPosition.x, currentPosition.y, width, height);
		Matrix4x4 modelMatrix = Math::GetTranslation(rect.left + rect.width / 2.0f, 
													 renderer->screenSize.y - rect.top - rect.height / 2.0f, 0) *
			Math::GetScale(rect.width / 2.0f, rect.height / 2.0f, 1);

		TextData newTextData = {};
		newTextData.color = color;
		newTextData.modelMatrix = modelMatrix;
		newTextData.sourceRect = sourceRect;
		context->layersText[layer][context->textPointers[layer]++] = newTextData;
		context->font = font;

		currentPosition.x += width;
	}
}

void Graphics2D::Release(Graphics2DContext *graphics2DContext)
{
	//Graphics::Release(&graphics2DContext->perModelbuffer);
	//Graphics::Release(&graphics2DContext->colorBuffer);
	//Graphics::Release(&graphics2DContext->spriteBuffer);
	//
	//Graphics::Release(&graphics2DContext->whiteTexture);
	//Graphics::Release(&graphics2DContext->fontTexture[0]);
	//Graphics::Release(&graphics2DContext->fontTexture[1]);
}

void Graphics2D::CommitRender(Graphics2DContext *graphics2DContext)
{
	Renderer *renderer = graphics2DContext->renderer;
	Texture *texture = NULL;
	for (uint32 layer = 0; layer < ARRAYSIZE(graphics2DContext->layers); ++layer)
	{
		Graphics::BindPipeline(renderer, graphics2DContext->program);
		for (uint32 i = 0; i < graphics2DContext->spritePointers[layer]; ++i)
		{
			SpriteData *data = &graphics2DContext->layers[layer][i];
			texture = data->texture;
			if (texture)
			{
				Graphics::BindTexture(renderer, texture);
			}
			else
			{
				Graphics::BindTexture(renderer, &graphics2DContext->whiteTexture);
			}

			Graphics::BindConstantBuffer(renderer, &graphics2DContext->colorBuffer, &data->color, COLOR_SLOT);
			Graphics::BindConstantBuffer(renderer, &graphics2DContext->spriteBuffer, &data->sourceRect, SPRITE_SLOT);
			PerModelData perModelData = {};
			perModelData.modelMatrix = data->modelMatrix;
			perModelData.instancingOn = false;
			Graphics::BindConstantBuffer(renderer, &graphics2DContext->perModelbuffer, &perModelData, MODEL_SLOT);
			Graphics::RenderModel(renderer, graphics2DContext->screenQuad);
		}

		Graphics::BindPipeline(renderer, graphics2DContext->textProgram);
		Graphics::BindTexture(renderer, &graphics2DContext->fontTexture[graphics2DContext->font]);
		for (uint32 i = 0; i < graphics2DContext->textPointers[layer]; ++i)
		{
			TextData *data = &graphics2DContext->layersText[layer][i];
			Graphics::BindConstantBuffer(renderer, &graphics2DContext->colorBuffer, &data->color, COLOR_SLOT);
			Graphics::BindConstantBuffer(renderer, &graphics2DContext->spriteBuffer, &data->sourceRect, SPRITE_SLOT);
			PerModelData perModelData = {};
			perModelData.modelMatrix = data->modelMatrix;
			perModelData.instancingOn = false;
			Graphics::BindConstantBuffer(renderer, &graphics2DContext->perModelbuffer, &perModelData, MODEL_SLOT);
			Graphics::RenderModel(renderer, graphics2DContext->screenQuad);
		}
	}
	if (texture)
	{
		//Graphics::UnbindTexture(renderer);
	}
}

void Graphics2D::ClearRender(Graphics2DContext *graphics2DContext)
{
	for (uint32 i = 0; i < ARRAYSIZE(graphics2DContext->spritePointers); ++i)
	{
		graphics2DContext->spritePointers[i] = 0;
		graphics2DContext->textPointers[i] = 0;
	}
}

