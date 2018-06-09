#include "Engine/Utils/TextureCompressor.h"
#include <stdio.h>
#include <iostream>

namespace NovaEngine {
	Texture* NovaEngine::loadCompressedImage(const char* filename) {
		FILE *pFile = fopen(filename, "rb");
		if (!pFile)
			return nullptr;
		GLuint info[4];

		fread(info, 4, 4, pFile);
		int width = info[0];
		int height = info[1];
		ImageFormat compressedFormat = (ImageFormat)info[2];
		int size = info[3];

		unsigned char *pData = (unsigned char*)malloc(size);
		fread(pData, size, 1, pFile);
		fclose(pFile);

		Texture* tex = new Texture2D(width, height, compressedFormat, pData);
		return tex;
	}

	void NovaEngine::compressTexture(Renderer* renderer, Texture* tex, const char* file_out) {
		if(!tex || tex == nullptr) return;
		int bpp = tex->getBitsPerPixel();
		if(bpp == -1) return;
		unsigned char* data = (unsigned char*)tex->getData(0);

		printf("Texture BPP: %d\r\n", bpp);

		ImageFormat format;
		if (bpp == 24) {
			format = ImageFormat::CompressedRGB;
		} else if (bpp == 32) {
			format = ImageFormat::CompressedRGBA;
		} else {
			return;
		}

		Texture* compTex = new Texture2D(tex->getWidth(), tex->getHeight(), format, data);
		compTex->setBitsPerPixel(bpp);
		renderer->setTexture(compTex, 0);

		GLint compressedSize;
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED_IMAGE_SIZE, &compressedSize);

		if ((compressedSize > 0) && (compressedSize < 100000000)) {
			printf("Texture can be compressed !\r\n");
			unsigned char* compressedBytes = (unsigned char*)malloc(sizeof(unsigned char) * compressedSize);
			glGetCompressedTexImage(GL_TEXTURE_2D, 0, compressedBytes);

			FILE *pFile = fopen(file_out, "wb");
			if (!pFile)
				return;

			unsigned int info[4];

			info[0] = compTex->getWidth();
			info[1] = compTex->getHeight();
			info[2] = (int)compTex->getFormat();
			info[3] = compressedSize;

			fwrite(info, 4, 4, pFile);
			fwrite(compressedBytes, compressedSize, 1, pFile);
			fclose(pFile);

			free(compressedBytes);
			printf("Texture has been compressed !\r\n");
		} else {
			printf("Texture doest not need compression.\r\n");
		}
	}

	bool NovaEngine::saveCompresseedImage(Texture* tex, const char* filename) {
		return false;
	}
}