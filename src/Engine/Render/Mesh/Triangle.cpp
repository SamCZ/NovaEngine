#include "Engine/Render/Mesh/Triangle.h"

namespace NovaEngine {
	Triangle::Triangle() {
	}

	glm::vec3 Triangle::computeTriangleNormal(glm::vec3& p1, glm::vec3& p2, glm::vec3& p3) {
		glm::vec3 U = p2 - p1;
		glm::vec3 V = p3 - p1;
		float x = (U.y * V.z) - (U.z * V.y);
		float y = (U.z * V.x) - (U.x * V.z);
		float z = (U.x * V.y) - (U.y * V.x);
		return glm::normalize(glm::vec3(x, y, z));
	}

}