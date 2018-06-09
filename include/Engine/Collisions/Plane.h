#ifndef C_PLANE_H
#define C_PLANE_H

#include <glm/glm.hpp>

namespace NovaEngine {
	enum class Side {
		None,
		Positive,
		Negative
	};

	class Plane {
	private:
		glm::vec3 _normal;
		float _constant;
	public:
		Plane();
		Plane(const glm::vec3& normal, const float constant);
		Plane(const glm::vec3& normal);

		void setNormal(const glm::vec3& normal);
		void setNormal(float x, float y, float z);
		glm::vec3& getNormal();
		void setConstant(float constant);
		float getConstant() const;
		glm::vec3 getClosestPoint(const glm::vec3& point);
		glm::vec3 reflect(const glm::vec3& point);
		float pseudoDistance(const glm::vec3& point);
		Side whichSide(const glm::vec3& point);
		bool isOnPlane(const glm::vec3& point);
		void setOriginNormal(const glm::vec3& origin, const glm::vec3& normal);

	};
}

#endif // !C_PLANE_H