#pragma once
#include "Core\Math.h"

namespace Util
{
	Vector4 HEXtoRGB(uint32 color);
	uint32 RGBtoHEX(Vector4 color);
	Vector4 HSVtoRGB(float h, float s, float v, float a = 1.0f);
	Vector4 HSVtoRGB(Vector4 hsvColor);
	Vector4 RGBtoHSV(Vector4 color);
	void ColorToHexChar(uint32 color, char *num, uint32 size);

	void PrintToString(char *destStr, uint32 size, char *format, ...);
	void IntToString(char *string, uint32 size, int val);
	void FloatToString(char *string, uint32 size, float val);


	bool PointInRect(Rect rect, Vector2 point);

}