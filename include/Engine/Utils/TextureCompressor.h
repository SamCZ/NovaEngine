#ifndef TEXTURE_COMPRESSOR_H
#define TEXTURE_COMPRESSOR_H

#include "Engine/Render/Renderer.h"

namespace NovaEngine {
	Texture* loadCompressedImage(const char* filename);
	void compressTexture(Renderer* renderer, Texture* tex, const char* file_out);
	bool saveCompresseedImage(Texture* tex, const char* filename);
}

#endif // !TEXTURE_COMPRESSOR_H
