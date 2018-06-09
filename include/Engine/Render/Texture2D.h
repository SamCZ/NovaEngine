#ifndef TEXTURE_2D_H
#define TEXTURE_2D_H

#include "Engine/Render/Texture.h"

namespace NovaEngine {
	class Texture2D : public Texture {
	protected:
		inline Texture2D(const int width, const int height, ImageFormat format, const std::vector<void*> pixels) :Texture(width, height, format, pixels) {}
	public:
		inline Texture2D(const int width, const int height, ImageFormat format) : Texture(width, height, format) {

		}
		inline Texture2D(const int width, const int height, ImageFormat format, void* pixels) : Texture(width, height, format, pixels) {

		}

		inline TextureType getType() {
			return TextureType::Texture2D;
		}
	};
}

#endif // !TEXTURE_2D_H