#include "Bark.h"
#include "PerlinNoise.h"
#include <cstdlib>

float Rand()
{
	return (rand() % 1000) / 1000.0f;
}

static float Noise(PerlinNoise *noise, uint32 x, uint32 y, uint32 z, float baseFreq)
{
	float f = baseFreq;
	float a = 0.5f;
	float a1 = Perlin::Noise3D(noise, x * f, y * f, z * f) * a;
	f *= 2;
	a *= 0.5f;
	float a2 = Perlin::Noise3D(noise, x * f, y * f, z * f) * a;
	f *= 2;
	a *= 0.5f;
	float a3 = Perlin::Noise3D(noise, x * f, y * f, z * f) * a;
	f *= 2;
	a *= 0.5f;
	float a4 = Perlin::Noise3D(noise, x * f, y * f, z * f) * a;
	f *= 2;
	a *= 0.5f;
	float a5 = Perlin::Noise3D(noise, x * f, y * f, z * f) * a;
	float s = a1 + a2 + a3 + a4 + a5;
	return s;
}

void FLBark::Init(Bark *bark, uint32 countX, uint32 countY, uint32 countZ)
{
	PerlinNoise perlin;
	Perlin::Init(&perlin);
	uint32 layerCellCount = countX * countY;
	uint32 cellCount = layerCellCount * countZ;
	bark->cells = (Cell *)malloc(sizeof(Cell) * cellCount);
	bark->cellsCopy = (Cell *)malloc(sizeof(Cell) * cellCount);
	bark->countX = countX;
	bark->countY = countY;
	bark->countZ = countZ;

	for (uint32 z = 0; z < countZ; ++z)
	{
		for (uint32 y = 0; y < countY; ++y)
		{
			for (uint32 x = 0; x < countX; ++x)
			{
				Cell *cell = GetCell(bark, x, y, z);
				for (uint32 i = 0; i < 6; ++i)
				{
					cell->connectionStrenghts[i] = 0.0f;
				}
				for (uint32 i = 0; i < 3; ++i)
				{
					cell->connectionStrengthsBottom[i] = 0.0f;
					cell->connectionStrengthsTop[i] = 0.0f;
				}
			}
		}
	}

	float angularDist = PI2 / countX;
	for (uint32 z = 0; z < countZ; ++z)
	{
		float radius = countX / PI2 + z * 0.8f;
		for (uint32 y = 0; y < countY; ++y)
		{
			uint32 offsetSwitch = (y + z) % 2;
			float offset = offsetSwitch * angularDist / 2.0f;
			float offsetY = (z % 2) * 1.0f;
			for (uint32 x = 0; x < countX; ++x)
			{
				float angle = ((float) x / (float) countX) * PI2 + offset;
				Cell *cell = GetCell(bark, x, y, z);
				float xPos = Math::Cos(angle) * radius;
				float yPos = (float)y + offsetY;
				float zPos = Math::Sin(angle) * radius;

				cell->position = Vector3(xPos, yPos, zPos);
				cell->originalPosition = cell->position;
				cell->velocity = Vector3(0,0,0);

				if (offsetSwitch == 0)
				{
					cell->connections[0] = GetCell(bark, x - 1, y + 1, z);
					cell->connections[1] = GetCell(bark, x, y + 1, z);
					cell->connections[3] = GetCell(bark, x, y - 1, z);
					cell->connections[4] = GetCell(bark, x - 1, y - 1, z);

					cell->connectionsBottom[0] = GetCell(bark, x, y + 1, z - 1);
					cell->connectionsBottom[1] = GetCell(bark, x, y, z - 1);
					cell->connectionsBottom[2] = GetCell(bark, x - 1, y, z - 1);

					cell->connectionsTop[0] = GetCell(bark, x - 1, y, z + 1);
					cell->connectionsTop[1] = GetCell(bark, x, y, z + 1);
					cell->connectionsTop[2] = GetCell(bark, x, y - 1, z + 1);
				}
				else
				{
					cell->connections[0] = GetCell(bark, x, y + 1, z);
					cell->connections[1] = GetCell(bark, x + 1, y + 1, z);
					cell->connections[3] = GetCell(bark, x + 1, y - 1, z);
					cell->connections[4] = GetCell(bark, x, y - 1, z);

					cell->connectionsBottom[0] = GetCell(bark, x, y + 1, z - 1);
					cell->connectionsBottom[1] = GetCell(bark, x + 1, y, z - 1);
					cell->connectionsBottom[2] = GetCell(bark, x, y, z - 1);

					cell->connectionsTop[0] = GetCell(bark, x, y, z + 1);
					cell->connectionsTop[1] = GetCell(bark, x + 1, y, z + 1);
					cell->connectionsTop[2] = GetCell(bark, x, y - 1, z + 1);
				}
				cell->connections[2] = GetCell(bark, x + 1, y, z);
				cell->connections[5] = GetCell(bark, x - 1, y, z);

				float strengthSigma = bark->strengthSigma;
				float interLayerStrength = 0.1f;
				float layerStrength = 0.1f;
				for (uint32 i = 0; i < 6; ++i)
				{
					if (cell->connections[i] && cell->connectionStrenghts[i] == 0)
					{
						float strength = Noise(&perlin, x, y, (z * i) % 255, bark->baseFreq) * strengthSigma + bark->strengthBase;
						cell->connectionStrenghts[i] = strength;
						cell->connections[i]->connectionStrenghts[(i + 3) % 6] = strength;
					}
				}
				for (uint32 i = 0; i < 3; ++i)
				{
					if (cell->connectionsBottom[i] && cell->connectionStrengthsBottom[i] == 0)
					{
						float strength = Noise(&perlin, x, (y * i) % 255, z, bark->baseFreq) * strengthSigma + bark->strengthBase;
						cell->connectionStrengthsBottom[i] = strength;
						cell->connectionsBottom[i]->connectionStrengthsTop[(i + 2) % 3] = strength;
					}
					if (cell->connectionsTop[i] && cell->connectionStrengthsTop[i] == 0)
					{
						float strength = Noise(&perlin, (x * i) % 255, y, z, bark->baseFreq) * strengthSigma + bark->strengthBase;
						cell->connectionStrengthsTop[i] = strength;
						cell->connectionsTop[i]->connectionStrengthsBottom[(i + 1) % 3] = strength;
					}
				}
				Cell *copyCell = GetCellForWriting(bark, x, y, z);
				*copyCell = *cell;
			}
		}
	}
}

void FLBark::Reinit(Bark *bark)
{
	PerlinNoise perlin;
	Perlin::Init(&perlin);

	uint32 layerCellCount = bark->countX * bark->countY;
	uint32 cellCount = layerCellCount * bark->countZ;

	for (uint32 z = 0; z < bark->countZ; ++z)
	{
		for (uint32 y = 0; y < bark->countY; ++y)
		{
			for (uint32 x = 0; x < bark->countX; ++x)
			{
				Cell *cell = GetCell(bark, x, y, z);
				for (uint32 i = 0; i < 6; ++i)
				{
					cell->connectionStrenghts[i] = 0.0f;
				}
				for (uint32 i = 0; i < 3; ++i)
				{
					cell->connectionStrengthsBottom[i] = 0.0f;
					cell->connectionStrengthsTop[i] = 0.0f;
				}
			}
		}
	}

	float angularDist = PI2 / bark->countX;
	for (uint32 z = 0; z < bark->countZ; ++z)
	{
		float radius = bark->countX / PI2 + z * 0.8f;
		for (uint32 y = 0; y < bark->countY; ++y)
		{
			uint32 offsetSwitch = (y + z) % 2;
			float offset = offsetSwitch * angularDist / 2.0f;
			float offsetY = (z % 2) * 1.0f;
			for (uint32 x = 0; x < bark->countX; ++x)
			{
				float angle = ((float)x / (float)bark->countX) * PI2 + offset;
				Cell *cell = GetCell(bark, x, y, z);
				float xPos = Math::Cos(angle) * radius;
				float yPos = (float)y + offsetY;
				float zPos = Math::Sin(angle) * radius;

				cell->position = Vector3(xPos, yPos, zPos);
				cell->originalPosition = cell->position;
				cell->velocity = Vector3(0, 0, 0);

				if (offsetSwitch == 0)
				{
					cell->connections[0] = GetCell(bark, x - 1, y + 1, z);
					cell->connections[1] = GetCell(bark, x, y + 1, z);
					cell->connections[3] = GetCell(bark, x, y - 1, z);
					cell->connections[4] = GetCell(bark, x - 1, y - 1, z);

					cell->connectionsBottom[0] = GetCell(bark, x, y + 1, z - 1);
					cell->connectionsBottom[1] = GetCell(bark, x, y, z - 1);
					cell->connectionsBottom[2] = GetCell(bark, x - 1, y, z - 1);

					cell->connectionsTop[0] = GetCell(bark, x - 1, y, z + 1);
					cell->connectionsTop[1] = GetCell(bark, x, y, z + 1);
					cell->connectionsTop[2] = GetCell(bark, x, y - 1, z + 1);
				}
				else
				{
					cell->connections[0] = GetCell(bark, x, y + 1, z);
					cell->connections[1] = GetCell(bark, x + 1, y + 1, z);
					cell->connections[3] = GetCell(bark, x + 1, y - 1, z);
					cell->connections[4] = GetCell(bark, x, y - 1, z);

					cell->connectionsBottom[0] = GetCell(bark, x, y + 1, z - 1);
					cell->connectionsBottom[1] = GetCell(bark, x + 1, y, z - 1);
					cell->connectionsBottom[2] = GetCell(bark, x, y, z - 1);

					cell->connectionsTop[0] = GetCell(bark, x, y, z + 1);
					cell->connectionsTop[1] = GetCell(bark, x + 1, y, z + 1);
					cell->connectionsTop[2] = GetCell(bark, x, y - 1, z + 1);
				}
				cell->connections[2] = GetCell(bark, x + 1, y, z);
				cell->connections[5] = GetCell(bark, x - 1, y, z);

				float strengthSigma = bark->strengthSigma;
				float interLayerStrength = 0.1f;
				float layerStrength = 0.1f;
				for (uint32 i = 0; i < 6; ++i)
				{
					if (cell->connections[i] && cell->connectionStrenghts[i] == 0)
					{
						float strength = Noise(&perlin, x, y, (z * i) % 255, bark->baseFreq) * strengthSigma + bark->strengthBase;
						cell->connectionStrenghts[i] = strength;
						cell->connections[i]->connectionStrenghts[(i + 3) % 6] = strength;
					}
				}
				for (uint32 i = 0; i < 3; ++i)
				{
					if (cell->connectionsBottom[i] && cell->connectionStrengthsBottom[i] == 0)
					{
						float strength = Noise(&perlin, x, (y * i) % 255, z, bark->baseFreq) * strengthSigma + bark->strengthBase;
						cell->connectionStrengthsBottom[i] = strength;
						cell->connectionsBottom[i]->connectionStrengthsTop[(i + 2) % 3] = strength;
					}
					if (cell->connectionsTop[i] && cell->connectionStrengthsTop[i] == 0)
					{
						float strength = Noise(&perlin, (x * i) % 255, y, z, bark->baseFreq) * strengthSigma + bark->strengthBase;
						cell->connectionStrengthsTop[i] = strength;
						cell->connectionsTop[i]->connectionStrengthsBottom[(i + 1) % 3] = strength;
					}
				}
				Cell *copyCell = GetCellForWriting(bark, x, y, z);
				*copyCell = *cell;
			}
		}
	}
}


Vector3 GetTensionInPlane(Bark *bark, int32 x, int32 y, int32 z)
{
	Cell *cell = FLBark::GetCell(bark, x, y, z);
	Vector3 tension(0, 0, 0);
	bool cond = bark->boundedLayer ? z > 0 : true;
	for (uint32 i = 0; i < 6; ++i)
	{
		if (cell->connections[i])
		{
			Vector3 distance = cell->connections[i]->position - cell->position;
			float magnitude = Math::Length(distance);
			distance = Math::Normalize(distance) * (magnitude - 1.0f);
			tension += distance;
			if (cond && Math::Length(distance) > cell->connectionStrenghts[i])
			{
				Cell *writeCell = FLBark::GetCellForWriting(bark, x, y, z);
				writeCell->connections[i] = NULL;
			}
		}
	}
	return tension;
}

Vector3 GetTensionTop(Bark *bark, int32 x, int32 y, int32 z)
{
	Cell *cell = FLBark::GetCell(bark, x, y, z);
	Vector3 tension(0, 0, 0);
	bool cond = bark->boundedLayer ? z > 0 : true;
	for (uint32 i = 0; i < 3; ++i)
	{
		if (cell->connectionsTop[i])
		{
			Vector3 distance = cell->connectionsTop[i]->position - cell->position;
			float magnitude = Math::Length(distance);
			distance = Math::Normalize(distance) * (magnitude - 1.0f);
			tension += distance;
			if (cond && Math::Length(distance) > cell->connectionStrengthsTop[i])
			{
				Cell *writeCell = FLBark::GetCellForWriting(bark, x, y, z);
				writeCell->connectionsTop[i] = NULL;
			}
		}
	}
	//Vector3 normal = Math::Normalize(cell->originalPosition);
	//tension = tension - normal * Math::Dot(normal, tension);
	return tension;
}

Vector3 GetTensionBottom(Bark *bark, int32 x, int32 y, int32 z)
{
	Cell *cell = FLBark::GetCell(bark, x, y, z);
	bool bottomLayer = z == 0;
	Vector3 tension(0, 0, 0);
	bool cond = bark->boundedLayer ? z > 1 : true;
	for (uint32 i = 0; i < 3; ++i)
	{
		if (cell->connectionsBottom[i])
		{
			Vector3 distance = cell->connectionsBottom[i]->position - cell->position;
			float magnitude = Math::Length(distance);
			distance = Math::Normalize(distance) * (magnitude - 1.0f);
			tension += distance;
			if (cond && Math::Length(distance) > cell->connectionStrengthsBottom[i])
			{
				Cell *writeCell = FLBark::GetCellForWriting(bark, x, y, z);
				writeCell->connectionsBottom[i] = NULL;
			}
		}
	}
	if (bottomLayer)
	{
		tension = (cell->originalPosition - cell->position) * 1.5f;
		//Vector3 normal = Math::Normalize(cell->originalPosition);
		//tension = tension - normal * Math::Dot(normal, tension);
		//Vector3 normalTension = -GetTensionTop(bark, x, y, z);
		//normalTension = normal * Math::Dot(normal, normalTension);
		//tension += normalTension;
	}
	//Vector3 normal = Math::Normalize(cell->originalPosition);
	//tension = tension - normal * Math::Dot(normal, tension);
	return tension;
}



#include <stdio.h>
#include <windows.h>
void FLBark::Update(Bark *bark, float dt)
{
	float maxY = -1000;
	float minY = 1000;
	for (uint32 z = 0; z < bark->countZ; ++z)
	{
		for (uint32 y = 0; y < bark->countY; ++y)
		{
			for (uint32 x = 0; x < bark->countX; ++x)
			{
				Cell *cell = GetCell(bark, x, y, z);
				Cell *writeCell = GetCellForWriting(bark, x, y, z);

				Vector3 tensionPlane = GetTensionInPlane(bark, x, y, z);
				Vector3 tensionBottom = GetTensionBottom(bark, x, y, z);
				Vector3 tensionTop = GetTensionTop(bark, x, y, z);

				Vector3 totalTension = tensionPlane * bark->intraLayerStiffness + 
					tensionBottom * bark->interLayerStiffness + tensionTop * bark->interLayerStiffness;
				Vector3 dampForce = -cell->velocity * bark->dampStr;
				Vector3 totalForce = dampForce + totalTension;

				writeCell->velocity += totalForce * dt;
				writeCell->position += writeCell->velocity * dt;

				Vector3 normal = cell->originalPosition;
				normal.y = 0;
				normal = Math::Normalize(normal);
				Vector3 growthVector = normal * dt * 0.4f;
				writeCell->position += growthVector;
				writeCell->originalPosition += growthVector;

				maxY = Math::Max(cell->position.y, maxY);
				minY = Math::Min(cell->position.y, minY);
			}
		}
	}

	for (uint32 z = 0; z < bark->countZ; ++z)
	{
		for (uint32 y = 0; y < bark->countY; ++y)
		{
			for (uint32 x = 0; x < bark->countX; ++x)
			{
				Cell *cell = GetCell(bark, x, y, z);
				Cell *writeCell = GetCellForWriting(bark, x, y, z);

				*cell = *writeCell;
			}
		}
	}
}

Cell *FLBark::GetCell(Bark *bark, int32 x, int32 y, int32 z)
{
	if (y >= (int32)bark->countY || y < 0 || z < 0 || z >= (int32)bark->countZ)
	{
		return NULL;
	}
	if (x < 0)
	{
		x += bark->countX;
	}
	x = x % bark->countX;
	return &bark->cells[z * bark->countX * bark->countY + y * bark->countX + x];
}

Cell *FLBark::GetCellForWriting(Bark *bark, int32 x, int32 y, int32 z)
{
	if (y >= (int32)bark->countY || y < 0 || z < 0 || z >= (int32)bark->countZ)
	{
		return NULL;
	}
	if (x < 0)
	{
		x += bark->countX;
	}
	x = x % bark->countX;
	return &bark->cellsCopy[z * bark->countX * bark->countY + y * bark->countX + x];
}
