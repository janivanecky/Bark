#pragma once
#include "Core\Def.h"

struct FileData;

enum VertexAttributes
{
	POSITION_NORMAL = 0,
	POSITION_TEXCOORD = 1,
	POSITION_NORMAL_TEXCOORD
};

namespace OBJLoader
{
	void GetMaterialCount(FileData *fileData, uint32 *materialCount);
	void LoadMaterials(FileData *fileData, float *materials, char *materialDictionary);

	void GetSizes(FileData *fileData, uint32 *vertexCount, uint32 *indexCount);
	void Load(FileData *fileData, float *vertices, uint16 *indices, VertexAttributes attributes, uint32 stride, uint32 indexOffset, bool reverseX = false);
	void LoadWithMaterial(FileData *fileData, float *vertices, uint16 *indices, VertexAttributes attributes, uint32 stride, uint32 indexOffset,
						  char *materialDictionary, uint32 materialsCount, bool reverseX = false);
};