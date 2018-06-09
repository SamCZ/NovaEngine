#ifndef GLM_UTILS_H
#define GLM_UTILS_H

#include "glm/glm.hpp"

namespace NovaEngine {
	/*long hash_ivec2(const glm::ivec2& v) {
		long A = (unsigned long)(v.x >= 0 ? 2 * (long)v.x : -2 * (long)v.x - 1);
		long B = (unsigned long)(v.y >= 0 ? 2 * (long)v.y : -2 * (long)v.y - 1);
		long C = (long)((A >= B ? A * A + A + B : A + B * B) / 2);
		return v.x < 0 && v.y < 0 || v.x >= 0 && v.y >= 0 ? C : -C - 1;
	}*/
	inline long hash_ivec2(int x, int y) {
		long A = (unsigned long)(x >= 0 ? 2 * (long)x : -2 * (long)x - 1);
		long B = (unsigned long)(y >= 0 ? 2 * (long)y : -2 * (long)y - 1);
		long C = (long)((A >= B ? A * A + A + B : A + B * B) / 2);
		return x < 0 && y < 0 || x >= 0 && y >= 0 ? C : -C - 1;
	}
}

#endif // !GLM_UTILS_H