#pragma once

#define PERLIN_SIZE 256

struct PerlinNoise
{
	int p[PERLIN_SIZE];
};

namespace Perlin
{
	void Init(PerlinNoise *noise);
	float Noise3D(PerlinNoise *noise, float x, float y, float z);
}