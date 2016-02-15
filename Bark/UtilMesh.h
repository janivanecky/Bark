#pragma once

#include "Core\Def.h"

struct Mesh
{
	float *vertices;
	uint32 vertexCount;
	uint32 vertexStride;
	uint16 *indices;
	uint32 indexCount;
};

namespace UtilMesh
{
	void MakeScreenQuad(Mesh *mesh);
	void MakeParticleQuad(Mesh *mesh);
	void MakeGrass(Mesh *mesh);
	void MakeFloor(Mesh *mesh);
	void MakeCube(Mesh *mesh, float edgeSize);
	void MakeCubeWithNormals(Mesh *mesh, float edgeSize);

	void Release(Mesh *mesh);
}