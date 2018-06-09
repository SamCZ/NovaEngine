#ifndef TEXTURE_CUBEMAP_H
#define TEXTURE_CUBEMAP_H

#include "Engine/Render/Texture.h"

namespace NovaEngine {
	class TextureCubeMap : public Texture {
	public:
		inline TextureCubeMap(const int width, const int height, ImageFormat format, const std::vector<void*> pixels) :Texture(width, height, format, pixels) { }

		inline TextureType getType() {
			return TextureType::CubeMap;
		}
	};
}

#endif // !TEXTURE_CUBEMAP_H