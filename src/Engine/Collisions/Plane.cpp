#include "Engine/Collisions/Plane.h"

namespace NovaEngine {
	Plane::Plane() : _normal(glm::vec3(0, 0, 0)), _constant(0.0f) {}

	Plane::Plane(const glm::vec3& normal, const float constant) : _normal(normal), _constant(constant) {}

	Plane::Plane(const glm::vec3& normal) : _normal(normal), _constant(0.0f) {}

	void Plane::setNormal(const glm::vec3& normal) {
		_normal = normal;
	}

	void Plane::setNormal(float x, float y, float z) {
		_normal.x = x;
		_normal.y = y;
		_normal.z = z;
	}

	glm::vec3& Plane::getNormal() {
		return _normal;
	}

	void Plane::setConstant(float constant) {
		_constant = constant;
	}

	float Plane::getConstant() const {
		return _constant;
	}

	glm::vec3 Plane::getClosestPoint(const glm::vec3& point) {
		float t = (_constant - glm::dot(_normal, point)) / glm::dot(_normal, _normal);
		return (_normal * t) + point;
	}

	glm::vec3 Plane::reflect(const glm::vec3& point) {
		float d = pseudoDistance(point);
		return ((_normal * -1.0f) * d * 2.0f) + point;
	}

	float Plane::pseudoDistance(const glm::vec3& point) {
		return glm::dot(_normal, point) - _constant;
	}

	Side Plane::whichSide(const glm::vec3& point) {
		float dis = pseudoDistance(point);
		if (dis < 0) {
			return Side::Negative;
		} else if (dis > 0) {
			return Side::Positive;
		} else {
			return Side::None;
		}
	}

	bool Plane::isOnPlane(const glm::vec3& point) {
		float dist = pseudoDistance(point);
		if (dist < FLT_EPSILON && dist > -FLT_EPSILON)
			return true;
		else
			return false;
	}

	void Plane::setOriginNormal(const glm::vec3& origin, const glm::vec3& normal) {
		_normal = normal;
		_constant = normal.x * origin.x + normal.y * origin.y + normal.z * origin.z;
	}

}
