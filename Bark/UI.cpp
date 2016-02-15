#include "UI.h"
#include "Util.h"
#include "Graphics2D.h"
#include "main.h"

static float UI_SCALE = 1.0f;
#define NORMAL_TEXT_SIZE 16.0f * UI_SCALE
#define TITLE_TEXT_SIZE (NORMAL_TEXT_SIZE * 1.1f)
#define VERTICAL_PADDING 5.0f * UI_SCALE
#define START_PADDING_X 10.0f * UI_SCALE
#define START_PADDING_Y 7.0f * UI_SCALE
#define ELEMENT_WIDTH 200.0f * UI_SCALE
#define ELEMENT_HEIGHT 50.0f * UI_SCALE

#define SLIDER_HEIGHT 2.0f
#define SLIDER_CONTROLLER_WIDTH 10.0f
#define SLIDER_CONTROLLER_HEIGHT 16.0f

#define BG_COLOR			Vector4(1,0,176.0f / 255.0f,0.5f)
#define FG_COLOR_MAIN		Vector4(1,1,1,1)//Vector4(0,0,0,1)
#define FG_COLOR_SECONDARY	Vector4(0,0,0,1)

Vector4 ColorFromSettings(UISettings *settings, ElementColor color)
{
	return settings->colors[color];
}

enum Align
{
	LEFT,
	RIGHT
};

Vector2 OriginFromAlign[] = {
	Vector2(0,0),
	Vector2(1,0)
};

Vector2 DrawText(UIContext *context, Vector2 position, char *text, float textSize, ElementColor color,
						  Align align = LEFT)
{
	UISettings *settings = &context->settings;
	Vector4 textColor = ColorFromSettings(settings, color);
	Graphics2D::DrawText(context->g2DContext, position, OriginFromAlign[align], text, textSize,
						 textColor, settings->activeFont);
	position.y += textSize + VERTICAL_PADDING;
	return position;
}

Vector4 Hover(Vector4 color)
{
	if (color.x < 0.25f || color.y < 0.25f || color.z < 0.25f)
	{
		color.x += 0.25f;
		color.y += 0.25f;
		color.z += 0.25f;
	}
	else
	{
		color.x -= 0.25f;
		color.y -= 0.25f;
		color.z -= 0.25f;
	}
	return color;
}

void DrawElementBG(UIContext *context, Vector2 position, 
				   float elementWidth = ELEMENT_WIDTH, float elementHeight = ELEMENT_HEIGHT)
{
	Rect bgRect(position.x, position.y, elementWidth, elementHeight);
	Color bgColor = ColorFromSettings(&context->settings, BGColor);
	if (Util::PointInRect(bgRect, context->input->touchPosition))
	{
		bgColor = Hover(bgColor);
	}
	Graphics2D::Draw(context->g2DContext, bgRect, bgColor);
}

Rect GetUIElementRect(Vector2 position, float width = ELEMENT_WIDTH, float height = ELEMENT_HEIGHT)
{
	return Rect(position.x, position.y, width, height);
}

void UI::InitContext(UIContext *context, Graphics2DContext *g2DContext, UISettings *settings)
{
	context->g2DContext = g2DContext;
	context->active = -1;
	context->hot = -1;
	context->sliderActive[0] = false;
	context->sliderActive[1] = false;
	UI_SCALE = g2DContext->renderer->screenSize.y / 600.0f * 0.8f;

	if (settings)
	{
		context->settings = *settings;
	}
	else
	{
		context->settings.activeFont = 0;
		context->settings.colors[0] = BG_COLOR;
		context->settings.colors[1] = FG_COLOR_MAIN;
		context->settings.colors[2] = FG_COLOR_SECONDARY;
	}

}

bool Deselect(UIContext *context, int32 ID, bool isMouseOver)
{
	if (context->active == ID && !isMouseOver && context->input->leftButtonPressed)
	{
		context->active = -1;
	}
	return context->active == ID;
}

void UpdateLayoutPosition(Layout *layout, float elementHeight)
{
	layout->position.y += elementHeight + VERTICAL_PADDING;
}

void UI::DrawLabelString(UIContext *context, Layout *layout, char *text, char *value)
{
	Vector2 position = layout->position;
	UISettings *settings = &context->settings;
	DrawElementBG(context, position);

	position.x += START_PADDING_X;
	position.y += START_PADDING_Y;

	position = DrawText(context, position, text, NORMAL_TEXT_SIZE, TextColorMain);
	position = DrawText(context, position, value, TITLE_TEXT_SIZE, TextColorSecondary);

	UpdateLayoutPosition(layout, ELEMENT_HEIGHT);
}

bool UI::DrawToggle(UIContext *context, Layout *layout, char *text, bool state, int32 ID, char *shortcut)
{
	Vector2 position = layout->position;
	UISettings *settings = &context->settings;
	DrawElementBG(context, position);

	Rect bgRect = GetUIElementRect(position);
	bool isMouseOver = Util::PointInRect(bgRect, context->input->touchPosition);
	bool hot = false;
	bool clicked = false;

	if (context->hot == ID)
	{
		if (!isMouseOver)
		{
			context->hot = -1;
		}
		else
		{
			hot = true;
			if (context->input->leftButtonPressed)
			{
				clicked = true;
				context->active = ID;
			}
		}
	}
	else
	{
		if (isMouseOver)
		{
			context->hot = ID;
			hot = true;
		}
	}
	Deselect(context, ID, isMouseOver);

	position.x += START_PADDING_X;
	position.y += START_PADDING_Y;

	position = DrawText(context, position, text, NORMAL_TEXT_SIZE, TextColorMain);
	char *toggleState = state ? "ON" : "OFF";
	ElementColor stateColor = hot || state ? TextColorMain : TextColorSecondary;
	DrawText(context, position, toggleState, TITLE_TEXT_SIZE, stateColor);

	if (shortcut)
	{
		Vector2 shortcutPosition = Vector2(bgRect.left + bgRect.width - START_PADDING_X, position.y);
		ElementColor shortcutColor = state ? TextColorMain : TextColorSecondary; 
		DrawText(context, shortcutPosition, shortcut, NORMAL_TEXT_SIZE, shortcutColor, RIGHT);
	}

	layout->position.y += ELEMENT_HEIGHT + VERTICAL_PADDING;

	return clicked;
}

float Slider(UIContext *context, uint32 ID, Vector2 position, float value, float from, float to, uint32 sliderID,
			float width = ELEMENT_WIDTH)
{
	UISettings *settings = &context->settings;
	float percentage = Math::Clamp((value - from) / (to - from), 0, 1);
	Rect sliderRect = Rect(position.x, position.y, width - 2.0f * START_PADDING_X, SLIDER_HEIGHT);
	Rect controllerRect = Rect(sliderRect.left - SLIDER_CONTROLLER_WIDTH / 2.0f + percentage * sliderRect.width,
							   position.y - SLIDER_CONTROLLER_HEIGHT / 2.0f + 1.0f,
							   SLIDER_CONTROLLER_WIDTH, SLIDER_CONTROLLER_HEIGHT);
	bool isMouseOver = Util::PointInRect(controllerRect, context->input->touchPosition);
	value = percentage;
	if (context->hot == ID && isMouseOver)
	{
		if (context->input->leftButtonPressed)
		{
			context->sliderActive[sliderID] = true;
			Vector2 rectCenter = Vector2(controllerRect.left + controllerRect.width / 2.0f,
										 controllerRect.top + controllerRect.height / 2.0f);
			context->clickDisplacement = context->input->touchPosition - rectCenter;
		}
	}
	if (context->hot == ID && context->sliderActive[sliderID])
	{
		Vector2 realPos = context->input->touchPosition - context->clickDisplacement;
		value = realPos.x;
		value = Math::Clamp((value - sliderRect.left) / sliderRect.width, 0, 1);
		if (!context->input->leftButtonDown)
		{
			context->sliderActive[sliderID] = false;
		}
	}

	ElementColor controllerColor = context->hot == ID && (isMouseOver || context->sliderActive[sliderID]) ? TextColorMain : TextColorSecondary;
	Graphics2D::Draw(context->g2DContext, sliderRect, ColorFromSettings(settings, TextColorSecondary), 1);
	Graphics2D::Draw(context->g2DContext, controllerRect, ColorFromSettings(settings, controllerColor), 1);

	value *= to - from;
	value += from;

	return value;
}

float UI::DrawSlider(UIContext *context, Layout *layout, char *text, float state, int32 ID, float from, float to)
{
	Vector2 position = layout->position;
	DrawElementBG(context, position);

	Rect bgRect = GetUIElementRect(position);

	position.x += START_PADDING_X;
	position.y += START_PADDING_Y;

	DrawText(context, position, text, NORMAL_TEXT_SIZE, TextColorMain);

	Vector2 valTextPosition = Vector2(bgRect.left + bgRect.width - START_PADDING_X, position.y);
	char buffer[20];
	Util::FloatToString(buffer, 20, state);
	DrawText(context, valTextPosition, buffer, NORMAL_TEXT_SIZE, TextColorMain, RIGHT);
	position.y += NORMAL_TEXT_SIZE + VERTICAL_PADDING * 2.0f;

	state = Slider(context, ID, position, state, from, to, 0);
	
	bool isMouseOver = Util::PointInRect(bgRect, context->input->touchPosition);
	if (context->hot == ID)
	{
		if (!isMouseOver && !context->input->leftButtonDown)
		{
			context->hot = -1;
			context->active = -1;
		}
		else
		{
			if (context->input->leftButtonPressed)
			{
				context->active = ID;
			}
			if (!context->input->leftButtonDown)
			{
				context->active = -1;
			}
		}
	}
	else
	{
		if (isMouseOver)
		{
			context->hot = ID;
		}
	}
	Deselect(context, ID, isMouseOver);

	UpdateLayoutPosition(layout, ELEMENT_HEIGHT);
	return state;
}

Vector4 UI::DrawColorPicker(UIContext *context, Layout *layout, char *text, Vector4 color, int32 ID)
{
	UISettings *settings = &context->settings;
	Vector2 position = layout->position;
	Vector2 startPosition = position;
	Vector4 returnColor = color;
	uint32 colorCount = 20;
	float pickerWidth = ELEMENT_WIDTH;
	float areaWidth = pickerWidth - 2.0f * START_PADDING_X;
	float colorWidth = areaWidth / colorCount;
	float colorHeight = colorWidth;
	float areaHeight = colorHeight * colorCount;

	position.x += START_PADDING_X;
	position.y += START_PADDING_Y;
	Vector2 titlePosition = position;
	position = DrawText(context, position, text, NORMAL_TEXT_SIZE, TextColorMain);

	char hexColor[14];
	Util::ColorToHexChar(Util::RGBtoHEX(color), hexColor, 14);
	DrawText(context, position, hexColor, NORMAL_TEXT_SIZE, TextColorSecondary);

	if (context->active == ID)
	{
		position.y += VERTICAL_PADDING * 5.0f;
		float state = Util::RGBtoHSV(color).z;

		Rect colorRect(position.x, position.y, colorWidth, colorHeight);
		for (uint32 y = 0; y < colorCount; ++y)
		{
			Rect currentRowRect = colorRect;
			for (uint32 x = 0; x < colorCount; ++x)
			{
				Vector4 color = Util::HSVtoRGB(1.0f / colorCount * x, 1.0f / colorCount * y, state, 1.0f);
				Graphics2D::Draw(context->g2DContext, currentRowRect, color, 1);
				if (Util::PointInRect(currentRowRect, context->input->touchPosition)
					&& context->input->leftButtonDown)
				{
					returnColor = color;
				}
				currentRowRect.left += colorWidth;
			}
			colorRect.top += colorHeight;
		}

		position.y += areaHeight + VERTICAL_PADDING * 3.0f;

		Vector4 hsvResult = Util::RGBtoHSV(returnColor);
		hsvResult.z = Slider(context, ID, position, hsvResult.z, 0, 1, 0);
		returnColor = Util::HSVtoRGB(hsvResult);

		position.y += SLIDER_CONTROLLER_HEIGHT * 0.75f + VERTICAL_PADDING;

		position = DrawText(context, position, "ALPHA", 0.8f * NORMAL_TEXT_SIZE, TextColorSecondary);

		returnColor.w = Slider(context, ID, position, returnColor.w, 0, 1, 1);

		position.y += SLIDER_CONTROLLER_HEIGHT * 0.5f + SLIDER_HEIGHT;
	}

	float maxHeight = position.y - startPosition.y + START_PADDING_Y;
	float pickerHeight = context->active == ID ? maxHeight : ELEMENT_HEIGHT;

	Rect pickerRect(startPosition.x, startPosition.y, pickerWidth, pickerHeight);
	bool isMouseOver = Util::PointInRect(pickerRect, context->input->touchPosition);
	Deselect(context, ID, isMouseOver);
	if (context->hot == ID)
	{
		if (!isMouseOver && !context->input->leftButtonDown)
		{
			context->hot = -1;
		}
		else
		{
			if (context->input->leftButtonPressed)
			{
				context->active = ID;
			}
		}
	}
	else
	{
		if (isMouseOver)
		{
			context->hot = ID;
		}
	}
	DrawElementBG(context, startPosition, pickerWidth, pickerHeight);

	layout->position.y += pickerHeight + VERTICAL_PADDING;
	return returnColor;
}

void UI::DrawLabelInt(UIContext *context, Layout *layout, char *text, int value)
{
	char valueString[10];
	Util::IntToString(valueString, 10, value);
	UI::DrawLabelString(context, layout, text, valueString);
}

void UI::DrawLabelFloat(UIContext *context, Layout *layout, char *text, float value)
{
	char valueString[10];
	Util::FloatToString(valueString, 10, value);
	UI::DrawLabelString(context, layout, text, valueString);
}

void UI::InitLayout(Layout *layout, Vector2 position, Anchor anchor)
{
	layout->position = position;
	if (anchor == ANCHOR_RIGHT)
	{
		layout->position.x -= ELEMENT_WIDTH;
	}
}
