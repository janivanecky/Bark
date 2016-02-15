#include "OBJLoader.h"
#include "Core\Math.h"
#include "Core\String.h"
#include "PlatformFunctions.h"
#include <cstring>

#define ADVANCE_TO_NEWLINE(x, counter, maxSize) for(;*x != '\n' && *x != '\r' && counter < maxSize; x++, counter++); for(;(*x == '\n' || *x == '\r') && counter < maxSize; x++, counter++);
#define ADVANCE_TO_ENDOFLINE(x, counter, maxSize) for(;*x != '\n' && *x != '\r' && counter < maxSize; x++, counter++); 
#define ADVANCE_TILL_AFTER(x, y, counter, maxSize) for(;*x != y && counter < maxSize; x++, counter++); x++; counter++;
#define ADVANCE_WHILE(x, y, counter, maxSize) for(;*x == y && counter < maxSize; x++, counter++);
#define ADVANCE_ONE(x, counter) x++; counter++;
#define ADVANCE_X(x, counter, xJumps) x += xJumps; counter += xJumps;

#define TO_NUM(x) ((x) - 48)

void GetMaxIndices(FileData *fileData, uint32 *vertexIndex, uint32 *texcoordIndex, uint32 *normalIndex, uint32 *totalIndices)
{
	uint8 *fileInput = (uint8 *)fileData->data;
	uint32 fileSize = fileData->size;
	uint32 currentPosition = 0;
	uint32 indexCountLocal = 0;
	uint32 maxVertexIndex = 0;
	uint32 maxTexcoordIndex = 0;
	uint32 maxNormalIndex = 0;
	while (currentPosition < fileSize)
	{
		if (*fileInput == 'f')
		{
			indexCountLocal += 3;
			ADVANCE_ONE(fileInput, currentPosition);
			for (uint32 i = 0; i < 3; i++)
			{
				ADVANCE_WHILE(fileInput, ' ', currentPosition, fileSize);
				uint32 localVertexIndex = 0;
				while (*fileInput != '/')
				{
					localVertexIndex = 10 * localVertexIndex + TO_NUM(*fileInput);
					ADVANCE_ONE(fileInput, currentPosition);
				}
				maxVertexIndex = Math::Max(localVertexIndex, maxVertexIndex);
				ADVANCE_ONE(fileInput, currentPosition);
				uint32 localTexcoordIndex = 0;
				while (*fileInput != '/')
				{
					localTexcoordIndex = 10 * localTexcoordIndex + TO_NUM(*fileInput);
					ADVANCE_ONE(fileInput, currentPosition);
				}
				maxTexcoordIndex = Math::Max(localTexcoordIndex, maxTexcoordIndex );
				ADVANCE_ONE(fileInput, currentPosition);
				uint32 localNormalIndex = 0;
				while (*fileInput != ' ' && *fileInput != '\n' && *fileInput != '\r')
				{
					localNormalIndex = 10 * localNormalIndex + TO_NUM(*fileInput);
					ADVANCE_ONE(fileInput, currentPosition);
				}
				maxNormalIndex = Math::Max(localNormalIndex, maxNormalIndex);
			}
			ADVANCE_TO_NEWLINE(fileInput, currentPosition, fileSize);
		}
		else
		{
			ADVANCE_TO_NEWLINE(fileInput, currentPosition, fileSize);
		}
	}
	*vertexIndex = maxVertexIndex;
	*texcoordIndex = maxTexcoordIndex;
	*normalIndex = maxNormalIndex;
	*totalIndices = indexCountLocal;
}

void OBJLoader::GetSizes(FileData *fileData, uint32 *vertexCount, uint32 *indexCount)
{
	uint32 maxVertexIndex, maxTexcoordIndex, maxNormalIndex;
	GetMaxIndices(fileData, &maxVertexIndex, &maxTexcoordIndex, &maxNormalIndex, indexCount);
	maxTexcoordIndex = Math::Max((uint32)maxTexcoordIndex, (uint32)1);
	maxNormalIndex = Math::Max((uint32)maxNormalIndex, (uint32)1);

	uint32 **uniqueVertices = new uint32 *[maxVertexIndex];
	for (uint32 i = 0; i < maxVertexIndex; ++i)
	{
		uniqueVertices[i] = new uint32[20];
	}

	uint32 *perVertexListCount = new uint32[maxVertexIndex];
	memset(perVertexListCount, 0, sizeof(uint32) * maxVertexIndex);

	uint8 *fileInput = (uint8 *)fileData->data;
	uint32 currentPosition = 0;
	uint32 fileSize = fileData->size;
	while (currentPosition < fileSize)
	{
		if (*fileInput == 'f')
		{
			ADVANCE_ONE(fileInput, currentPosition);
			for (uint32 i = 0; i < 3; i++)
			{
				ADVANCE_WHILE(fileInput, ' ', currentPosition, fileSize);
				uint32 vertexIndex = 0;
				while (*fileInput != '/')
				{
					vertexIndex = 10 * vertexIndex + TO_NUM(*fileInput);
					ADVANCE_ONE(fileInput, currentPosition);
				}
				if (vertexIndex > 0)
				{
					--vertexIndex;
				}
				ADVANCE_ONE(fileInput, currentPosition);
				uint32 texcoordIndex = 0;
				while (*fileInput != '/')
				{
					texcoordIndex = 10 * texcoordIndex + TO_NUM(*fileInput);
					ADVANCE_ONE(fileInput, currentPosition);
				}
				if (texcoordIndex > 0)
				{
					--texcoordIndex;
				}
				ADVANCE_ONE(fileInput, currentPosition);
				uint32 normalIndex = 0;
				while (*fileInput != ' ' && *fileInput != '\n' && *fileInput != '\r')
				{
					normalIndex = 10 * normalIndex + TO_NUM(*fileInput);
					ADVANCE_ONE(fileInput, currentPosition);
				}
				if (normalIndex > 0)
				{
					--normalIndex;
				}
				//uniqueVertex[normalIndex + maxNormalIndex * texcoordIndex + maxNormalIndex * maxTexcoordIndex * vertexIndex] = 1;
				bool alreadyExists = false;
				uint32 listLength = perVertexListCount[vertexIndex];
				for (uint32 l = 0; l < listLength; l++)
				{
					if (uniqueVertices[vertexIndex][l * 2] == normalIndex &&
						uniqueVertices[vertexIndex][l * 2 + 1] == texcoordIndex)
					{
						alreadyExists = true;
					}
				}
				if (!alreadyExists)
				{
					uniqueVertices[vertexIndex][listLength * 2] = normalIndex;
					uniqueVertices[vertexIndex][listLength * 2 + 1] = texcoordIndex;
					++perVertexListCount[vertexIndex];
				}
			}
			ADVANCE_TO_NEWLINE(fileInput, currentPosition, fileSize);
		}
		else
		{
			ADVANCE_TO_NEWLINE(fileInput, currentPosition, fileSize);
		}
	}

	uint32 uniqueVerticesCount = 0;
	for (uint32 i = 0; i < maxVertexIndex; ++i)
	{
		uniqueVerticesCount += perVertexListCount[i];
		delete[] uniqueVertices[i];
	}

	delete[] uniqueVertices;
	delete[] perVertexListCount;
	*vertexCount = uniqueVerticesCount;
}

float ReadNextFloat(uint8 *fileInput, uint32 *positionsRead)
{
	bool fraction = false;
	float fractional = 10.0f;
	float sign = 1.0f;
	float result = 0.0f;
	uint32 positionCounter = 0;
	while (*fileInput != ' ' && *fileInput != '\n' && *fileInput != '\r')
	{
		if (*fileInput == '.')
		{
			fraction = true;
		}
		else if (*fileInput == '-')
		{
			sign = -1.0f;
		}
		else
		{
			if (fraction)
			{
				result = result + TO_NUM(*fileInput) / fractional;
				fractional *= 10.0f;
			}
			else
			{
				result = 10.0f * result + TO_NUM(*fileInput);
			}
		}
		ADVANCE_ONE(fileInput, positionCounter);
	}
	*positionsRead = positionCounter;
	result *= sign;
	return result;
}

void CopyVectors(float *src, float *dst, uint32 components)
{
	for (uint32 i = 0; i < components; i++)
	{
		*dst++ = *src++;
	}
}

uint32 floatCountFromAttributes[] = {6, 5, 8};

void OBJLoader::LoadWithMaterial(FileData *fileData, float *verticesOutput, uint16 *indicesOutput, VertexAttributes attributes, uint32 stride, 
								 uint32 indexOffset, char *materialDictionary, uint32 materialCount, bool reverseX)
{
	uint32 maxVertexIndex, maxTexcoordIndex, maxNormalIndex, totalIndices;

	GetMaxIndices(fileData, &maxVertexIndex, &maxTexcoordIndex, &maxNormalIndex, &totalIndices);

	float *vertices = new float[3 * maxVertexIndex];
	float *texcoords = new float[2 * maxTexcoordIndex];
	float *normals = new float[3 * maxNormalIndex];
	uint16 *indices = new uint16[totalIndices];

	uint8 *fileInput = (uint8 *)fileData->data;
	uint32 currentPosition = 0;
	uint32 fileSize = fileData->size;
	uint32 vertexIndex = 0;
	uint32 normalIndex = 0;
	uint32 texcoordIndex = 0;
	while (currentPosition < fileSize)
	{
		if (*fileInput == 'v')
		{
			ADVANCE_ONE(fileInput, currentPosition);
			if (*fileInput == ' ')
			{
				float position[3] = { 0, 0, 0 };
				for (uint32 i = 0; i < 3; i++)
				{
					ADVANCE_WHILE(fileInput, ' ', currentPosition, fileSize);
					uint32 jump = 0;
					position[i] = ReadNextFloat(fileInput, &jump);
					ADVANCE_X(fileInput, currentPosition, jump);
				}
				if (reverseX)
				{
					position[0] = -position[0];
				}
				CopyVectors(position, &vertices[3 * vertexIndex], 3);
				++vertexIndex;
				ADVANCE_TO_NEWLINE(fileInput, currentPosition, fileSize);
 			}
			else if (*fileInput == 'n')
			{
				ADVANCE_ONE(fileInput, currentPosition);
				float normal[3] = { 0, 0, 0 };
				for (uint32 i = 0; i < 3; i++)
				{
					ADVANCE_WHILE(fileInput, ' ', currentPosition, fileSize);
					uint32 jump = 0;
					normal[i] = ReadNextFloat(fileInput, &jump);
					ADVANCE_X(fileInput, currentPosition, jump);
				}
				CopyVectors(normal, &normals[3 * normalIndex], 3);
				++normalIndex;
				ADVANCE_TO_NEWLINE(fileInput, currentPosition, fileSize);
			}
			else if (*fileInput == 't')
			{
				ADVANCE_ONE(fileInput, currentPosition);
				float texcoord[2] = { 0, 0 };
				for (uint32 i = 0; i < 2; i++)
				{
					ADVANCE_WHILE(fileInput, ' ', currentPosition, fileSize);
					uint32 jump = 0;
					texcoord[i] = ReadNextFloat(fileInput, &jump);
					ADVANCE_X(fileInput, currentPosition, jump);
				}
				CopyVectors(texcoord, &texcoords[2 * texcoordIndex], 2);
				++texcoordIndex;
				ADVANCE_TO_NEWLINE(fileInput, currentPosition, fileSize);
			}
		}
		else
		{
			ADVANCE_TO_NEWLINE(fileInput, currentPosition, fileSize);
		}
	}
	maxTexcoordIndex = Math::Max((uint32)maxTexcoordIndex, (uint32)1);
	maxNormalIndex = Math::Max((uint32)maxNormalIndex, (uint32)1);

	uint32 **uniqueVertices = new uint32 *[maxVertexIndex];
	for (uint32 i = 0; i < maxVertexIndex; ++i)
	{
		uniqueVertices[i] = new uint32[30];
	}

	uint32 *perVertexListCount = new uint32[maxVertexIndex];
	memset(perVertexListCount, 0, sizeof(uint32) * maxVertexIndex);

	fileInput = (uint8 *)fileData->data;
	currentPosition = 0;
	uint32 currentMaterialIndex = 0;
	uint32 currentIndex = indexOffset;
	uint32 objStride = floatCountFromAttributes[attributes] * sizeof(float) + 1;
	uint32 floatPadding = 0;
	if (objStride < stride)
	{
		floatPadding = (stride - objStride) / sizeof(float);
	}
	while (currentPosition < fileSize)
	{
		if (String::Contains((char *)fileInput, "usemtl"))
		{
			ADVANCE_X(fileInput, currentPosition, 6);
			ADVANCE_WHILE(fileInput, ' ', currentPosition, fileSize);
			char *startOfNamePointer = (char *)fileInput;
			for (uint32 i = 0; i < materialCount; ++i)
			{
				if (String::Contains(startOfNamePointer, &materialDictionary[i * 10]))
				{
					currentMaterialIndex = i;
				}
			}
		}
		else if (*fileInput == 'f')
		{
			ADVANCE_ONE(fileInput, currentPosition);
			for (uint32 i = 0; i < 3; i++)
			{
				ADVANCE_WHILE(fileInput, ' ', currentPosition, fileSize);
				uint32 localVertexIndex = 0;
				while (*fileInput != '/')
				{
					localVertexIndex = 10 * localVertexIndex + TO_NUM(*fileInput);
					ADVANCE_ONE(fileInput, currentPosition);
				}
				if (localVertexIndex > 0)
				{
					--localVertexIndex;
				}
				ADVANCE_ONE(fileInput, currentPosition);
				uint32 localTexcoordIndex = 0;
				while (*fileInput != '/')
				{
					localTexcoordIndex = 10 * localTexcoordIndex + TO_NUM(*fileInput);
					ADVANCE_ONE(fileInput, currentPosition);
				}
				if (localTexcoordIndex > 0)
				{
					--localTexcoordIndex;
				}
				ADVANCE_ONE(fileInput, currentPosition);
				uint32 localNormalIndex = 0;
				while (*fileInput != ' ' && *fileInput != '\n' && *fileInput != '\r')
				{
					localNormalIndex = 10 * localNormalIndex + TO_NUM(*fileInput);
					ADVANCE_ONE(fileInput, currentPosition);
				}
				if (localNormalIndex > 0)
				{
					--localNormalIndex;
				}

				bool alreadyExists = false;
				uint32 listLength = perVertexListCount[localVertexIndex];
				for (uint32 l = 0; l < listLength; l++)
				{
					if (uniqueVertices[localVertexIndex][l * 3] == localNormalIndex &&
						uniqueVertices[localVertexIndex][l * 3 + 1] == localTexcoordIndex)
					{
						alreadyExists = true;
						*indicesOutput++ = uniqueVertices[localVertexIndex][l * 3 + 2];
					}
				}
				if (!alreadyExists)
				{
					uniqueVertices[localVertexIndex][listLength * 3] = localNormalIndex;
					uniqueVertices[localVertexIndex][listLength * 3 + 1] = localTexcoordIndex;
					uniqueVertices[localVertexIndex][listLength * 3 + 2] = currentIndex;
					++perVertexListCount[localVertexIndex];

					CopyVectors(&vertices[localVertexIndex * 3], verticesOutput, 3);
					verticesOutput += 3;
					if (attributes == POSITION_NORMAL || attributes == POSITION_NORMAL_TEXCOORD)
					{
						CopyVectors(&normals[localNormalIndex * 3], verticesOutput, 3);
						verticesOutput += 3;
					}
					if (attributes == POSITION_TEXCOORD || attributes == POSITION_NORMAL_TEXCOORD)
					{
						CopyVectors(&texcoords[localTexcoordIndex * 2], verticesOutput, 2);
						verticesOutput += 2;
					}
					float *materialPointer = verticesOutput++;
					*materialPointer = (float)currentMaterialIndex;
					verticesOutput += floatPadding;
					*indicesOutput++ = currentIndex++;
				}
			}
			ADVANCE_TO_NEWLINE(fileInput, currentPosition, fileSize);
		}
		else
		{
			ADVANCE_TO_NEWLINE(fileInput, currentPosition, fileSize);
		}
	}

	delete[] vertices;
	delete[] texcoords;
	delete[] normals;
	delete[] indices;

	for (uint32 i = 0; i < maxVertexIndex; ++i)
	{
		delete[] uniqueVertices[i];
	}
	delete[] uniqueVertices;
	delete[] perVertexListCount;
}

void OBJLoader::Load(FileData *fileData, float *verticesOutput, uint16 *indicesOutput, VertexAttributes attributes, uint32 stride, uint32 indexOffset, bool reverseX)
{
	uint32 maxVertexIndex, maxTexcoordIndex, maxNormalIndex, totalIndices;

	GetMaxIndices(fileData, &maxVertexIndex, &maxTexcoordIndex, &maxNormalIndex, &totalIndices);

	float *vertices = new float[3 * maxVertexIndex];
	float *texcoords = new float[2 * maxTexcoordIndex];
	float *normals = new float[3 * maxNormalIndex];
	uint16 *indices = new uint16[totalIndices];

	uint8 *fileInput = (uint8 *)fileData->data;
	uint32 currentPosition = 0;
	uint32 fileSize = fileData->size;
	uint32 vertexIndex = 0;
	uint32 normalIndex = 0;
	uint32 texcoordIndex = 0;
	while (currentPosition < fileSize)
	{
		if (*fileInput == 'v')
		{
			ADVANCE_ONE(fileInput, currentPosition);
			if (*fileInput == ' ')
			{
				float position[3] = { 0, 0, 0 };
				for (uint32 i = 0; i < 3; i++)
				{
					ADVANCE_WHILE(fileInput, ' ', currentPosition, fileSize);
					uint32 jump = 0;
					position[i] = ReadNextFloat(fileInput, &jump);
					ADVANCE_X(fileInput, currentPosition, jump);
				}
				if (reverseX)
				{
					position[0] = -position[0];
				}
				CopyVectors(position, &vertices[3 * vertexIndex], 3);
				++vertexIndex;
				ADVANCE_TO_NEWLINE(fileInput, currentPosition, fileSize);
 			}
			else if (*fileInput == 'n')
			{
				ADVANCE_ONE(fileInput, currentPosition);
				float normal[3] = { 0, 0, 0 };
				for (uint32 i = 0; i < 3; i++)
				{
					ADVANCE_WHILE(fileInput, ' ', currentPosition, fileSize);
					uint32 jump = 0;
					normal[i] = ReadNextFloat(fileInput, &jump);
					ADVANCE_X(fileInput, currentPosition, jump);
				}
				CopyVectors(normal, &normals[3 * normalIndex], 3);
				++normalIndex;
				ADVANCE_TO_NEWLINE(fileInput, currentPosition, fileSize);
			}
			else if (*fileInput == 't')
			{
				ADVANCE_ONE(fileInput, currentPosition);
				float texcoord[2] = { 0, 0 };
				for (uint32 i = 0; i < 2; i++)
				{
					ADVANCE_WHILE(fileInput, ' ', currentPosition, fileSize);
					uint32 jump = 0;
					texcoord[i] = ReadNextFloat(fileInput, &jump);
					ADVANCE_X(fileInput, currentPosition, jump);
				}
				CopyVectors(texcoord, &texcoords[2 * texcoordIndex], 2);
				++texcoordIndex;
				ADVANCE_TO_NEWLINE(fileInput, currentPosition, fileSize);
			}
		}
		else
		{
			ADVANCE_TO_NEWLINE(fileInput, currentPosition, fileSize);
		}
	}
	maxTexcoordIndex = Math::Max((uint32)maxTexcoordIndex, (uint32)1);
	maxNormalIndex = Math::Max((uint32)maxNormalIndex, (uint32)1);

	uint32 **uniqueVertices = new uint32 *[maxVertexIndex];
	for (uint32 i = 0; i < maxVertexIndex; ++i)
	{
		uniqueVertices[i] = new uint32[30];
	}

	uint32 *perVertexListCount = new uint32[maxVertexIndex];
	memset(perVertexListCount, 0, sizeof(uint32) * maxVertexIndex);

	fileInput = (uint8 *)fileData->data;
	currentPosition = 0;
	uint32 currentIndex = indexOffset;
	uint32 objStride = floatCountFromAttributes[attributes] * sizeof(float);
	uint32 floatPadding = 0;
	if (objStride < stride)
	{
		floatPadding = (stride - objStride) / sizeof(float);
	}
	while (currentPosition < fileSize)
	{
		if (*fileInput == 'f')
		{
			ADVANCE_ONE(fileInput, currentPosition);
			for (uint32 i = 0; i < 3; i++)
			{
				ADVANCE_WHILE(fileInput, ' ', currentPosition, fileSize);
				uint32 localVertexIndex = 0;
				while (*fileInput != '/')
				{
					localVertexIndex = 10 * localVertexIndex + TO_NUM(*fileInput);
					ADVANCE_ONE(fileInput, currentPosition);
				}
				if (localVertexIndex > 0)
				{
					--localVertexIndex;
				}
				ADVANCE_ONE(fileInput, currentPosition);
				uint32 localTexcoordIndex = 0;
				while (*fileInput != '/')
				{
					localTexcoordIndex = 10 * localTexcoordIndex + TO_NUM(*fileInput);
					ADVANCE_ONE(fileInput, currentPosition);
				}
				if (localTexcoordIndex > 0)
				{
					--localTexcoordIndex;
				}
				ADVANCE_ONE(fileInput, currentPosition);
				uint32 localNormalIndex = 0;
				while (*fileInput != ' ' && *fileInput != '\n' && *fileInput != '\r')
				{
					localNormalIndex = 10 * localNormalIndex + TO_NUM(*fileInput);
					ADVANCE_ONE(fileInput, currentPosition);
				}
				if (localNormalIndex > 0)
				{
					--localNormalIndex;
				}

				bool alreadyExists = false;
				uint32 listLength = perVertexListCount[localVertexIndex];
				for (uint32 l = 0; l < listLength; l++)
				{
					if (uniqueVertices[localVertexIndex][l * 3] == localNormalIndex &&
						uniqueVertices[localVertexIndex][l * 3 + 1] == localTexcoordIndex)
					{
						alreadyExists = true;
						*indicesOutput++ = uniqueVertices[localVertexIndex][l * 3 + 2];
					}
				}
				if (!alreadyExists)
				{
					uniqueVertices[localVertexIndex][listLength * 3] = localNormalIndex;
					uniqueVertices[localVertexIndex][listLength * 3 + 1] = localTexcoordIndex;
					uniqueVertices[localVertexIndex][listLength * 3 + 2] = currentIndex;
					++perVertexListCount[localVertexIndex];

					CopyVectors(&vertices[localVertexIndex * 3], verticesOutput, 3);
					verticesOutput += 3;
					if (attributes == POSITION_NORMAL || attributes == POSITION_NORMAL_TEXCOORD)
					{
						CopyVectors(&normals[localNormalIndex * 3], verticesOutput, 3);
						verticesOutput += 3;
					}
					if (attributes == POSITION_TEXCOORD || attributes == POSITION_NORMAL_TEXCOORD)
					{
						CopyVectors(&texcoords[localTexcoordIndex * 2], verticesOutput, 2);
						verticesOutput += 2;
					}
					verticesOutput += floatPadding;
					*indicesOutput++ = currentIndex++;
				}
			}
			ADVANCE_TO_NEWLINE(fileInput, currentPosition, fileSize);
		}
		else
		{
			ADVANCE_TO_NEWLINE(fileInput, currentPosition, fileSize);
		}
	}

 	delete[] vertices;
	delete[] texcoords;
	delete[] normals;
	delete[] indices;

	for (uint32 i = 0; i < maxVertexIndex; ++i)
	{
		delete[] uniqueVertices[i];
	}
	delete[] uniqueVertices;
	delete[] perVertexListCount;
}

void OBJLoader::GetMaterialCount(FileData *fileData, uint32 *materialCount)
{
	uint8 *fileInput = (uint8 *)fileData->data;
	uint32 fileSize = fileData->size;
	uint32 currentPosition = 0;
	uint32 materialCountLocal = 0;
	while (currentPosition < fileSize)
	{
		if (String::Contains((char *)fileInput, "newmtl"))
		{
			++materialCountLocal;
		}
		ADVANCE_TO_NEWLINE(fileInput, currentPosition, fileSize);
	}
	*materialCount = materialCountLocal;
}

void OBJLoader::LoadMaterials(FileData *fileData, float *materialsOutput, char *materialDictionary)
{
	uint8 *fileInput = (uint8 *)fileData->data;
	uint32 fileSize = fileData->size;
	uint32 currentPosition = 0;
	uint32 currentMaterial = 0;

	uint32 matCount = 0;
	GetMaterialCount(fileData, &matCount);
	float *ambientColors = new float[3 * matCount];
	float *diffuseColors = new float[3 * matCount];

	while (currentPosition < fileSize)
	{
		if (String::Contains((char *)fileInput, "newmtl"))
		{
			ADVANCE_X(fileInput, currentPosition, 6);
			ADVANCE_WHILE(fileInput, ' ', currentPosition, fileSize);
			char *startOfNamePointer = (char *)fileInput;
			uint32 startOfName = currentPosition;
			ADVANCE_TO_ENDOFLINE(fileInput, currentPosition, fileSize);
			uint32 endOfName = currentPosition;
			uint32 length = Math::Min((uint32)10, (uint32)(endOfName - startOfName + 1));
			String::CopySafe(startOfNamePointer, &materialDictionary[currentMaterial * 10], length);
			currentMaterial++;
		}
		
		if (String::Contains((char *)fileInput, "Ka"))
		{
			ADVANCE_X(fileInput, currentPosition, 2);
			float color[3] = { 0, 0, 0 };
			for (uint32 i = 0; i < 3; i++)
			{
				ADVANCE_WHILE(fileInput, ' ', currentPosition, fileSize);
				uint32 jump = 0;
				color[i] = ReadNextFloat(fileInput, &jump);
				ADVANCE_X(fileInput, currentPosition, jump);
			}
			CopyVectors(color, &ambientColors[(currentMaterial - 1) * 3], 3);
		}

		if (String::Contains((char *)fileInput, "Kd"))
		{
			ADVANCE_X(fileInput, currentPosition, 2);
			float color[3] = { 0, 0, 0 };
			for (uint32 i = 0; i < 3; i++)
			{
				ADVANCE_WHILE(fileInput, ' ', currentPosition, fileSize);
				uint32 jump = 0;
				color[i] = ReadNextFloat(fileInput, &jump);
				ADVANCE_X(fileInput, currentPosition, jump);
			}
			CopyVectors(color, &diffuseColors[(currentMaterial - 1) * 3], 3);
		}
		ADVANCE_TO_NEWLINE(fileInput, currentPosition, fileSize);
	}

	for (uint32 i = 0; i < matCount; ++i)
	{
		CopyVectors(&ambientColors[i * 3], materialsOutput, 3);
		materialsOutput += 3;
		CopyVectors(&diffuseColors[i * 3], materialsOutput, 3);
		materialsOutput += 3;
	}

	delete[] ambientColors;
	delete[] diffuseColors;
}