#ifndef RAY_H
#define RAY_H

#include "glm/glm.hpp"

namespace NovaEngine {
	class Ray {
	public:
		glm::vec3 origin;
		glm::vec3 direction;
		float limit;

		Ray();
		Ray(glm::vec3 origin, glm::vec3 dir);

		float intersects(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2);
	private:

	};
}

#endif // !RAY_H
