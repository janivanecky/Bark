#pragma once
#include "Graphics.h"

struct FileData;

namespace DDSLoader
{
	TextureDesc Load(FileData *file);
}