#include "PerlinNoise.h"
#include <cmath>
#include <cstdlib>

static void Normalize(float v[3])
{
	float s;
	s = sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	v[0] = v[0] / s;
	v[1] = v[1] / s;
	v[2] = v[2] / s;
}

static float g[12][3] = 
{
	{1,1,0},{-1,1,0},{1,-1,0},{-1,-1,0},
	{1,0,1},{-1,0,1},{1,0,-1},{-1,0,-1},
	{0,1,1},{0,-1,1},{0,1,-1},{0,-1,-1}
};

void Perlin::Init(PerlinNoise *noise)
{
	for (int i = 0; i < PERLIN_SIZE; ++i)
	{
		noise->p[i] = i;
	}

	for (int i = 0; i < PERLIN_SIZE; ++i)
	{
		int k = noise->p[i];
		int r = rand() % PERLIN_SIZE;
		noise->p[i] = noise->p[r];
		noise->p[r] = k;
	}
}

float Fmod(float x, float y)
{
	if (y == 0)
	{
		return 0;
	}
	while (x < 0)
	{
		x = x + y;
	}
	while (x >= y)
	{
		x -= y;
	}
	return x;
}

float Fade(float x)
{
	return x * x * x * (x * (x * 6 - 15) + 10); 
}

float Lerp(float a, float b, float t)
{
	return a * (1 - t) + b * t;
}

float Grad(int i, float x, float y, float z)
{
	i = i % 12;
	return g[i][0] * x + g[i][1] * y + g[i][2] * z;
}

float Perlin::Noise3D(PerlinNoise *noise, float x, float y, float z)
{
	x = Fmod(x, PERLIN_SIZE);
	y = Fmod(y, PERLIN_SIZE);
	z = Fmod(z, PERLIN_SIZE);

	int xi = (int)x;
	int yi = (int)y;
	int zi = (int)z;
	float xf = x - xi;
	float yf = y - yi;
	float zf = z - zi;

	float u = Fade(xf);
	float v = Fade(yf);
	float w = Fade(zf);

	int *p = noise->p;
	int aaa = p[(p[(p[xi] + yi) & 255] + zi) & 255];
	int aab = p[(p[(p[xi] + yi) & 255] + zi + 1) & 255];
	int aba = p[(p[(p[xi] + yi + 1) & 255] + zi) & 255];
	int abb = p[(p[(p[xi] + yi + 1) & 255] + zi + 1) & 255];
	int baa = p[(p[(p[xi + 1] + yi) & 255] + zi) & 255];
	int bab = p[(p[(p[xi + 1] + yi) & 255] + zi + 1) & 255];
	int bba = p[(p[(p[xi + 1] + yi + 1) & 255] + zi) & 255];
	int bbb = p[(p[(p[xi + 1] + yi + 1) & 255] + zi + 1) & 255];

	float gAAA = Grad(aaa, xf, yf, zf);
	float gAAB = Grad(aab, xf, yf, zf - 1);
	float gABA = Grad(aba, xf, yf - 1, zf);
	float gABB = Grad(abb, xf, yf - 1, zf - 1);
	float gBAA = Grad(baa, xf - 1, yf, zf);
	float gBAB = Grad(bab, xf - 1, yf, zf - 1);
	float gBBA = Grad(bba, xf - 1, yf - 1, zf);
	float gBBB = Grad(bbb, xf - 1, yf - 1, zf - 1);

	float z1 = Lerp(gAAA, gAAB, w);
	float z2 = Lerp(gABA, gABB, w);
	float z3 = Lerp(gBAA, gBAB, w);
	float z4 = Lerp(gBBA, gBBB, w);
	float y1 = Lerp(z1,z2,v);
	float y2 = Lerp(z3,z4,v);
	float x1 = Lerp(y1,y2,u);
	
	return x1 * 0.5f + 0.5f;
}