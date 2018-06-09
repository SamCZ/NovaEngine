#ifndef COLOR_RGBA_H
#define COLOR_RGBA_H

#include "glm/glm.hpp"

namespace NovaEngine {
	struct ColorRGBA {

		float r;
		float g;
		float b;
		float a;

		inline glm::vec4 toVec4() const {
			return glm::vec4(r, g, b, a);
		}
		inline glm::vec3 toVec3() const {
			return glm::vec3(r, g, b);
		}
	};

	ColorRGBA makeRGB(int r, int g, int b);
	ColorRGBA makeRGBA(int r, int g, int b, int a);
	ColorRGBA makeRGBf(float r, float g, float b);
	ColorRGBA makeRGBAf(float r, float g, float b, float a);
	ColorRGBA makeHEX(const char* hex);
}

#endif // !COLOR_RGBA_H