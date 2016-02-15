#pragma once
#include "Core\Math.h"

struct Cell
{
	Vector3 position;
	Vector3 originalPosition;
	Vector3 velocity;
	Cell *connections[6];
	float connectionStrenghts[6];
	Cell *connectionsBottom[3];
	float connectionStrengthsBottom[3];
	Cell *connectionsTop[3];
	float connectionStrengthsTop[3];
};

struct Bark
{
	Cell *cells;
	Cell *cellsCopy;
	uint32 countX;
	uint32 countY;
	uint32 countZ;

	float dampStr;
	float baseFreq;
	float strengthSigma;
	float strengthBase;
	float interLayerStiffness;
	float intraLayerStiffness;

	bool boundedLayer;
};

namespace FLBark
{
	void Init(Bark *bark, uint32 countX, uint32 countY, uint32 countZ);
	void Reinit(Bark *bark);
	void Update(Bark *bark, float dt);

	Cell *GetCell(Bark *bark, int32 x, int32 y, int32 z);
	Cell *GetCellForWriting(Bark *bark, int32 x, int32 y, int32 z);
}