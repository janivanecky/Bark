#include "Util.h"
#include <stdarg.h>
#include <string>

Vector4 Util::HEXtoRGB(uint32 color)
{
	float a = ((color & 0xFF000000) >> 24) / 255.0f;
	float r = ((color & 0x00FF0000) >> 16) / 255.0f;
	float g = ((color & 0x0000FF00) >> 8) / 255.0f;
	float b = (color & 0x000000FF) / 255.0f;
	return Vector4(r,g,b,a);
}

uint32 Util::RGBtoHEX(Vector4 color)
{
	uint32 a = (uint32)(0xFF * color.w) << 24;
	uint32 r = (uint32)(0xFF * color.x) << 16;
	uint32 g = (uint32)(0xFF * color.y) << 8;
	uint32 b = (uint32)(0xFF * color.z);
	uint32 result = a | r | g | b;
	return result;
}

Vector4 Util::HSVtoRGB(float h, float s, float v, float a)
{
	float min;
	float chroma;
	float hDash;
	float x;

	float r = 0, g = 0, b = 0;
	chroma = s * v;
	hDash = h * 6.0f;
	x = chroma * (1.0f - Math::Abs(Math::Fmod(hDash, 2.0f) - 1.0f));

	if (hDash < 1.0)
	{
		r = chroma;
		g = x;
	}
	else if (hDash < 2.0)
	{
		r = x;
		g = chroma;
	}
	else if (hDash < 3.0)
	{
		g = chroma;
		b = x;
	}
	else if (hDash < 4.0)
	{
		g = x;
		b = chroma;
	}
	else if (hDash < 5.0)
	{
		r = x;
		b = chroma;
	}
	else if (hDash <= 6.0)
	{
		r = chroma;
		b = x;
	}

	min = v - chroma;

	r += min;
	g += min;
	b += min;

	return Vector4(r, g, b, a);
}

Vector4 Util::HSVtoRGB(Vector4 col)
{
	Vector4 result = HSVtoRGB(col.x, col.y, col.z);
	result.w = col.w;
	return result;
}

Vector4 Util::RGBtoHSV(Vector4 color)
{
	Vector4 result;
	float cMax = Math::Max(color.x, Math::Max(color.y, color.z));
	float cMin = Math::Min(color.x, Math::Min(color.y, color.z));
	float delta = cMax - cMin;
	float h = 0;
	float coef = 1.0f / 6.0f;
	if (cMax == color.x)
	{
		h = coef * Math::Fmod((color.y - color.z) / delta, 6.0f);
	}
	if (cMax == color.y)
	{
		h = coef * ((color.y - color.x) / delta + 2.0f);
	}
	if (cMax == color.z)
	{
		h = coef * ((color.x - color.y) / delta + 4.0f);
	}

	float s = 0;
	if (cMax == 0)
	{
		s = 0;
	}
	else
	{
		s = delta / cMax;
	}

	float v = cMax;
	return Vector4(h,s,v,color.w);
}


void Util::PrintToString(char *destStr, uint32 size, char *format, ...)
{
	va_list argList;
	va_start(argList, format);
	vsprintf_s(destStr, size, format, argList);
	va_end(argList);
}


void Util::IntToString(char *string, uint32 size, int val)
{
	Util::PrintToString(string, size, "%d", val);
}

void Util::FloatToString(char *string, uint32 size, float val)
{
	Util::PrintToString(string, size, "%.2f", val);
}

void Util::ColorToHexChar(uint32 color, char *num, uint32 size)
{
	num[0] = '0';
	num[1] = 'x';
	Util::PrintToString(num + 2, size - 2, "%X", color);
}

bool Util::PointInRect(Rect rect, Vector2 point)
{
	return rect.top <= point.y && rect.left <= point.x &&
		rect.top + rect.height >= point.y && rect.left + rect.width >= point.x;
}