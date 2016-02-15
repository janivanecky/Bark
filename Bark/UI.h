#pragma once
#include "Core\Math.h"

struct Input;
struct Graphics2DContext;

enum ElementColor
{
	BGColor,
	TextColorMain,
	TextColorSecondary
};

struct UISettings
{
	Vector4 bgColor;
	Vector4 fgColorMain;
	Vector4 fgColorSecondary;
	Vector4 colors[3];
	uint32 activeFont;
};

struct UIContext
{
	int32 active;
	int32 hot;
	Input *input;
	Vector2 clickDisplacement;
	Graphics2DContext *g2DContext;

	bool sliderActive[2];

	UISettings settings;
};

struct Layout
{
	Vector2 position;
};

enum Anchor
{
	ANCHOR_LEFT,
	ANCHOR_RIGHT
};

namespace UI
{
	void InitContext(UIContext *context, Graphics2DContext *g2DContext, UISettings *setting);

	void InitLayout(Layout *layout, Vector2 position, Anchor anchor = ANCHOR_LEFT);
	void DrawLabelString(UIContext *context, Layout *layout, char *text, char *value);
	void DrawLabelInt(UIContext *context, Layout *layout, char *text, int value);
	void DrawLabelFloat(UIContext *context, Layout *layout, char *text, float value);

	Vector4 DrawColorPicker(UIContext *context, Layout *layout, char *text, Vector4 color, int32 ID);
	bool DrawToggle(UIContext *context, Layout *layout, char *text, bool state, int32 ID, char *shortcut = NULL);
	float DrawSlider(UIContext *context, Layout *layout, char *text, float state, int32 ID, float from, float to);
}