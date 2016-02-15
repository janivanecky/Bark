#include "UtilMesh.h"
#include <stdlib.h>
#include <memory>

void UtilMesh::MakeScreenQuad(Mesh *mesh)
{
	const float vertices[] =
	{
		-1.0f, -1.0f, 0.0f, 0.0f, 1.f,
		1.0f, -1.0f, 0.0f, 1.f, 1.f,
		1.0f, 1.0f, 0.0f, 1.f, 0.f,
		-1.0f, 1.0f, 0.0f, 0.f, 0.f,
	};

	uint32 stride = 5 * sizeof(float);

	uint16 indices[] =
	{
		2, 1, 0,
		0, 3, 2,
	};

	mesh->vertexCount = ARRAYSIZE(vertices);
	mesh->vertexStride = stride;
	mesh->indexCount = ARRAYSIZE(indices);
	mesh->indices = (uint16*)malloc(sizeof(uint16) * mesh->indexCount);
	mesh->vertices = (float*)malloc(sizeof(float) * mesh->vertexCount);

	memcpy(mesh->vertices, vertices, sizeof(vertices));
	memcpy(mesh->indices, indices, sizeof(indices));
}

void UtilMesh::MakeParticleQuad(Mesh *mesh)
{
	static const float vertices[] =
	{
		-1.0f, -1.0f, 0.0f, 0.0f, 0.f, -1, 3,
		1.0f, -1.0f, 0.0f, 0.f, 0.f, -1, 3,
		1.0f, 1.0f, 0.0f, 0.f, 0.f, -1, 3,
		-1.0f, 1.0f, 0.0f, 0.f, 0.f, -1, 3,
	};

	uint32 stride = 7 * sizeof(float);

	static uint32 indices[] =
	{
		0, 1, 2,
		2, 3, 0,
	};

	mesh->vertexCount = ARRAYSIZE(vertices);
	mesh->vertexStride = stride;
	mesh->indexCount = ARRAYSIZE(indices);
	mesh->indices = (uint16*)malloc(sizeof(uint16) * mesh->indexCount);
	mesh->vertices = (float*)malloc(sizeof(float) * mesh->vertexCount);

	memcpy(mesh->vertices, vertices, sizeof(vertices));
	memcpy(mesh->indices, indices, sizeof(indices));
}

void UtilMesh::MakeFloor(Mesh *mesh)
{
	float length = 100.0f;
	static const float vertices[] =
	{
		-length, 0, length, 0, 1, 0, 2,
		length, 0, length, 0, 1, 0, 2,
		length, 0, -length, 0, 1, 0, 2,
		-length, 0, -length, 0, 1, 0, 2,
	};

	uint32 stride = 7 * sizeof(float);

	static uint32 indices[] =
	{
		0, 1, 2,
		2, 3, 0,
	};

	mesh->vertexCount = ARRAYSIZE(vertices);
	mesh->vertexStride = stride;
	mesh->indexCount = ARRAYSIZE(indices);
	mesh->indices = (uint16*)malloc(sizeof(uint16) * mesh->indexCount);
	mesh->vertices = (float*)malloc(sizeof(float) * mesh->vertexCount);

	memcpy(mesh->vertices, vertices, sizeof(vertices));
	memcpy(mesh->indices, indices, sizeof(indices));
}

void UtilMesh::MakeGrass(Mesh *mesh)
{
	static const float vertices[] =
	{
		-0.15f, 0, 0, 0, 0, -1, 1,
		0.15f, 0, 0, 0, 0, -1, 1,
		0, 1, 0, 0, 0, -1, 1
	};

	uint32 stride = 7 * sizeof(float);

	static uint32 indices[] =
	{
		0, 1, 2
	};

	mesh->vertexCount = ARRAYSIZE(vertices);
	mesh->vertexStride = stride;
	mesh->indexCount = ARRAYSIZE(indices);
	mesh->indices = (uint16*)malloc(sizeof(uint16) * mesh->indexCount);
	mesh->vertices = (float*)malloc(sizeof(float) * mesh->vertexCount);

	memcpy(mesh->vertices, vertices, sizeof(vertices));
	memcpy(mesh->indices, indices, sizeof(indices));
}

void UtilMesh::MakeCube(Mesh *mesh, float edgeSize)
{
	float size = edgeSize;
	float vertices[] =
	{
		size, 0, 0,			// LBN 0
		0, 0, 0,			// RBN 1
		0, size, 0,			// RTN 2
		size, size, 0,		// LTN 3

		0, 0, size,			// RBF 4
		size, 0, size,		// LBF 5
		size, size, size,	// LTF 6
		0, size, size		// RTF 7
	};

	uint32 stride = 3 * sizeof(float);

	uint32 iLBN = 0;
	uint32 iRBN = 1;
	uint32 iRTN = 2;
	uint32 iLTN = 3;

	uint32 iRBF = 4;
	uint32 iLBF = 5;
	uint32 iLTF = 6;
	uint32 iRTF = 7;

	static uint32 indices[] =
	{
		// Front
		iRTN, iLTN, iLBN,
		iLBN, iRBN, iRTN,

		// Top
		iLTN, iRTN, iRTF,
		iRTF, iLTF, iLTN,

		// Back
		iRBF, iLBF, iLTF,
		iLTF, iRTF, iRBF,

		// Bottom
		iLBF, iRBF, iRBN,
		iRBN, iLBN, iLBF,

		// Right
		iRBN, iRBF, iRTF,
		iRTF, iRTN, iRBN,

		// Left
		iLBF, iLBN, iLTN,
		iLTN, iLTF, iLBF,
	};

	mesh->vertexCount = ARRAYSIZE(vertices);
	mesh->vertexStride = stride;
	mesh->indexCount = ARRAYSIZE(indices);
	mesh->indices = (uint16*)malloc(sizeof(uint16) * mesh->indexCount);
	mesh->vertices = (float*)malloc(sizeof(float) * mesh->vertexCount);

	memcpy(mesh->vertices, vertices, sizeof(vertices));
	memcpy(mesh->indices, indices, sizeof(indices));
}

void UtilMesh::MakeCubeWithNormals(Mesh *mesh, float edgeSize)
{
	float size = edgeSize;

	float vertices[] =
	{
		// Front
		0, size, 0,			// RTN 2
		size, size, 0,		// LTN 3
		size, 0, 0,			// LBN 0

		size, 0, 0,			// LBN 0
		0, 0, 0,			// RBN 1
		0, size, 0,			// RTN 2

		// Top
		size, size, 0,		// LTN 3
		0, size, 0,			// RTN 2
		0, size, size,		// RTF 7

		0, size, size,		// RTF 7
		size, size, size,	// LTF 6
		size, size, 0,		// LTN 3

		// Back
		0, 0, size,			// RBF 4
		size, 0, size,		// LBF 5
		size, size, size,	// LTF 6

		size, size, size,	// LTF 6
		0, size, size,		// RTF 7
		0, 0, size,			// RBF 4

		// Bottom
		size, 0, size,		// LBF 5
		0, 0, size,			// RBF 4
		0, 0, 0,			// RBN 1

		0, 0, 0,			// RBN 1
		size, 0, 0,			// LBN 0
		size, 0, size,		// LBF 5

		// Right
		0, 0, 0,			// RBN 1
		0, 0, size,			// RBF 4
		0, size, size,		// RTF 7

		0, size, size,	// RTF 7
		0, size, 0,			// RTN 2
		0, 0, 0,			// RBN 1

		// Left
		size, 0, size,		// LBF 5
		size, 0, 0,			// LBN 0
		size, size, 0,		// LTN 3

		size, size, 0,		// LTN 3
		size, size, size,	// LTF 6
		size, 0, size,		// LBF 5
	};


	uint32 stride = 3 * sizeof(float);

	static uint32 indices[] =
	{
		// Front
		0,1,2,
		3,4,5,
		// Top
		6,7,8,
		9,10,11,
	
		// Back
		12,13,14,
		15,16,17,
	
		// Bottom
		18,19,20,
		21,22,23,
	
		// Right
		24,25,26,
		27,28,29,
	
		// Left
		30,31,32,
		33,34,35,
	};


	mesh->vertexCount = ARRAYSIZE(vertices);
	mesh->vertexStride = stride;
	mesh->indexCount = ARRAYSIZE(indices);
	mesh->indices = (uint16*)malloc(sizeof(uint16) * mesh->indexCount);
	mesh->vertices = (float*)malloc(sizeof(float) * mesh->vertexCount);

	memcpy(mesh->vertices, vertices, sizeof(vertices));
	memcpy(mesh->indices, indices, sizeof(indices));
}

void UtilMesh::Release(Mesh *mesh)
{
	free(mesh->indices);
	free(mesh->vertices);
	mesh->vertexCount = 0;
	mesh->indexCount = 0;
}
