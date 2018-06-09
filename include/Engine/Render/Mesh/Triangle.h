#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "glm/glm.hpp"
#include "glm/geometric.hpp"

namespace NovaEngine {
	class Triangle {
	public:
		Triangle();

		static glm::vec3 computeTriangleNormal(glm::vec3& p1, glm::vec3& p2, glm::vec3& p3);
	};
}

#endif // !TRIANGLE_H
