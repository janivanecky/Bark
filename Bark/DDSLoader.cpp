#include "main.h"
#include "DDSLoader.h"

struct DDS_PIXELFORMAT {
  uint32 dwSize;
  uint32 dwFlags;
  uint32 dwFourCC;
  uint32 dwRGBBitCount;
  uint32 dwRBitMask;
  uint32 dwGBitMask;
  uint32 dwBBitMask;
  uint32 dwABitMask;
};

struct DDS_HEADER{
  uint32 dwSize;
  uint32 dwFlags;
  uint32 dwHeight;
  uint32 dwWidth;
  uint32 dwPitchOrLinearSize;
  uint32 dwDepth;
  uint32 dwMipMapCount;
  uint32 dwReserved1[11];
  DDS_PIXELFORMAT ddspf;
  uint32 dwCaps;
  uint32 dwCaps2;
  uint32 dwCaps3;
  uint32 dwCaps4;
  uint32 dwReserved2;
};

TextureDesc DDSLoader::Load(FileData *file)
{
	uint8 *fileData = (uint8 *)file->data;
	uint32 magicNumber = *((uint32 *)fileData);
	fileData += sizeof(magicNumber);
	DDS_HEADER *header = (DDS_HEADER *)(fileData);
	fileData += sizeof(DDS_HEADER);
	TextureDesc result;
	result.width = header->dwWidth;
	result.height = header->dwHeight;
	result.mipCount = header->dwMipMapCount;
	result.data = (void *)fileData;
	return result;
}