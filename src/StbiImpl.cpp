/**
 * Here are the implementations of the STBI functions
 * Do not include this file, use pch.h instead
 */

#include "pch.h"
#include "Allocator.hpp"

#define STBI_MALLOC(sz) ISF_MALLOC(sz)
#define STBI_REALLOC(p, newsz) ISF_REALLOC(p, newsz)
#define STBI_FREE(p) ISF_FREE(p)

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
