#include "Engine/Collisions/BoundingBox.h"
#include "Engine/Collisions/Collidable.h"
#include "Engine/Collisions/Ray.h"
#include <limits>
#include "glm/gtx/matrix_decompose.hpp"
#include "glm/gtx/quaternion.hpp"
#include <glm/gtx/transform.hpp>
#include "Engine/Utils/Buffers.h"
#include <iostream>
#include "Engine/Collisions/Plane.h"

namespace NovaEngine {
	BoundingBox::BoundingBox() : _center() {
		checkSide = 0;
	}

	BoundingBox::~BoundingBox() {
	}

	BoundingBox::BoundingBox(BoundingBox * box) {
		_center = box->getCenter();
		_xExtent = box->getXExtent();
		_yExtent = box->getYExtent();
		_zExtent = box->getZExtent();
		checkSide = 0;
	}

	BoundingBox::BoundingBox(glm::vec3 min, glm::vec3 max) : BoundingBox() {
		setMinMax(min, max);
		checkSide = 0;
	}

	BoundingBox::BoundingBox(glm::vec3 center, float xExtent, float yExtent, float zExtent) {
		_center = center;
		_xExtent = xExtent;
		_yExtent = yExtent;
		_zExtent = zExtent;
		checkSide = 0;
	}

	void BoundingBox::computeFromMesh(Mesh* mesh) {
		float _minX = std::numeric_limits<float>::infinity();
		float _minY = std::numeric_limits<float>::infinity();
		float _minZ = std::numeric_limits<float>::infinity();

		float _maxX = -std::numeric_limits<float>::infinity();
		float _maxY = -std::numeric_limits<float>::infinity();
		float _maxZ = -std::numeric_limits<float>::infinity();

		VertexBuffer* indexVertexBuffer = mesh->getVertexBuffer(MeshBuffer::Type::Index);
		VertexBuffer* posVertexBuffer = mesh->getVertexBuffer(MeshBuffer::Type::Position);

		if (indexVertexBuffer == nullptr) {
			std::cout << "NO INDEX" << std::endl;
			return;
		}

		FloatBuffer* vertices = static_cast<FloatBuffer*>(posVertexBuffer->getBuffer());

		for (int i = 0; i < vertices->size() / 3; i++) {
			glm::vec3 v = glm::vec3(vertices->get(i * 3 + 0), vertices->get(i * 3 + 1), vertices->get(i * 3 + 2));
			if (v.x < _minX) {
				_minX = v.x;
			}
			if (v.x > _maxX) {
				_maxX = v.x;
			}

			if (v.y < _minY) {
				_minY = v.y;
			}
			if (v.y > _maxY) {
				_maxY = v.y;
			}

			if (v.z < _minZ) {
				_minZ = v.z;
			}
			if (v.z > _maxZ) {
				_maxZ = v.z;
			}
		}

		_center = glm::vec3(_minX + _maxX, _minY + _maxY, _minZ + _maxZ);
		_center *= 0.5f;

		_xExtent = _maxX - _center.x;
		_yExtent = _maxY - _center.y;
		_zExtent = _maxZ - _center.z;
	}

	void BoundingBox::computeFromPoints(std::vector<glm::vec3>& points, glm::mat4* transform) {
		float _minX = std::numeric_limits<float>::infinity();
		float _minY = std::numeric_limits<float>::infinity();
		float _minZ = std::numeric_limits<float>::infinity();

		float _maxX = -std::numeric_limits<float>::infinity();
		float _maxY = -std::numeric_limits<float>::infinity();
		float _maxZ = -std::numeric_limits<float>::infinity();

		bool useTransform = transform != nullptr;
		for (int i = 0; i < points.size(); i++) {
			glm::vec3 v = useTransform ? *transform * glm::vec4(points[i], 1.0) : points[i];

			if (v.x < _minX) {
				_minX = v.x;
			}
			if (v.x > _maxX) {
				_maxX = v.x;
			}

			if (v.y < _minY) {
				_minY = v.y;
			}
			if (v.y > _maxY) {
				_maxY = v.y;
			}

			if (v.z < _minZ) {
				_minZ = v.z;
			}
			if (v.z > _maxZ) {
				_maxZ = v.z;
			}
		}

		_center = glm::vec3(_minX + _maxX, _minY + _maxY, _minZ + _maxZ);
		_center *= 0.5f;

		_xExtent = _maxX - _center.x;
		_yExtent = _maxY - _center.y;
		_zExtent = _maxZ - _center.z;

		//setMinMax(glm::vec3(_minX, _minY, _minZ), glm::vec3(_maxX, _maxY, _maxZ));
	}

	void BoundingBox::computeFromPoints(float * points, int size, glm::mat4 * transform) {
		float _minX = std::numeric_limits<float>::infinity();
		float _minY = std::numeric_limits<float>::infinity();
		float _minZ = std::numeric_limits<float>::infinity();

		float _maxX = -std::numeric_limits<float>::infinity();
		float _maxY = -std::numeric_limits<float>::infinity();
		float _maxZ = -std::numeric_limits<float>::infinity();

		bool useTransform = transform != nullptr;
		glm::vec3 point = glm::vec3();
		for (int i = 0; i < size / 3; i++) {
			point.x = points[i * 3 + 0];
			point.y = points[i * 3 + 1];
			point.z = points[i * 3 + 2];
			glm::vec3 v = useTransform ? *transform * glm::vec4(point, 1.0) : point;

			if (v.x < _minX) {
				_minX = v.x;
			}
			if (v.x > _maxX) {
				_maxX = v.x;
			}

			if (v.y < _minY) {
				_minY = v.y;
			}
			if (v.y > _maxY) {
				_maxY = v.y;
			}

			if (v.z < _minZ) {
				_minZ = v.z;
			}
			if (v.z > _maxZ) {
				_maxZ = v.z;
			}
		}

		_center = glm::vec3(_minX + _maxX, _minY + _maxY, _minZ + _maxZ);
		_center *= 0.5f;

		_xExtent = _maxX - _center.x;
		_yExtent = _maxY - _center.y;
		_zExtent = _maxZ - _center.z;
	}

	int BoundingBox::collideWithRay(Ray ray, CollisionResults& results) {
		glm::vec3 diff = ray.origin - _center;
		glm::vec3 direction = ray.direction;

		_clipTemp[0] = 0;
		_clipTemp[1] = std::numeric_limits<float>::infinity();

		float saveT0 = _clipTemp[0];
		float saveT1 = _clipTemp[1];

		bool notEntirelyClipped =
			clip(+direction.x, -diff.x - _xExtent, _clipTemp) &&
			clip(-direction.x, +diff.x - _xExtent, _clipTemp) &&

			clip(+direction.y, -diff.y - _yExtent, _clipTemp) &&
			clip(-direction.y, +diff.y - _yExtent, _clipTemp) &&

			clip(+direction.z, -diff.z - _zExtent, _clipTemp) &&
			clip(-direction.z, +diff.z - _zExtent, _clipTemp);
		if (notEntirelyClipped && (_clipTemp[0] != saveT0 || _clipTemp[1] != saveT1)) {
			if (_clipTemp[1] > _clipTemp[0]) {
				glm::vec3 point0 = (ray.direction * _clipTemp[0]) + ray.origin;
				glm::vec3 point1 = (ray.direction * _clipTemp[1]) + ray.origin;

				CollisionResult result;
				result.isNull = false;
				result.contactPoint = point0;
				result.distance = _clipTemp[0];
				results.addCollision(result);

				CollisionResult result2;
				result2.isNull = false;
				result2.contactPoint = point1;
				result2.distance = _clipTemp[1];
				results.addCollision(result2);
			}
			glm::vec3 point = (ray.direction * _clipTemp[0]) + ray.origin;
			CollisionResult result;
			result.isNull = false;
			result.contactPoint = point;
			result.distance = _clipTemp[0];
			results.addCollision(result);
			return 1;
		}
		return 0;
	}

	BoundingBox* BoundingBox::transform(glm::vec3 location, glm::vec3 rotation, glm::vec3 scale) {

		glm::vec3 center = _center * scale;

		glm::mat4 rotationMat = glm::mat4();
		rotationMat *= glm::rotate(glm::radians(rotation.x), glm::vec3(1, 0, 0));
		rotationMat *= glm::rotate(glm::radians(rotation.y), glm::vec3(0, 1, 0));
		rotationMat *= glm::rotate(glm::radians(rotation.z), glm::vec3(0, 0, 1));

		//glm::quat rotationQuat = glm::toQuat(rotationMat);
		//glm::mat3 rotationMat3 = glm::mat3(rotationMat);

		glm::mat3 rot = glm::mat3(rotationMat);

		for (int i = 0; i < 3; i++) {
			for (int a = 0; a < 3; a++) {
				rot[i][a] = glm::abs(rot[i][a]);
			}
		}

		center = rot * center;
		center += location;

		glm::vec3 vect1 = glm::vec3(_xExtent * glm::abs(scale.x), _yExtent * glm::abs(scale.y), _zExtent * glm::abs(scale.z));
		glm::vec3 vect2 = rot * vect1;

		float xExtent = glm::abs(vect2.x);
		float yExtent = glm::abs(vect2.y);
		float zExtent = glm::abs(vect2.z);

		return new BoundingBox(center, xExtent, yExtent, zExtent);

		/*glm::vec4 newCenter4 = trans * glm::vec4(_center, 1.0f);
		glm::vec3 newCenter = newCenter4;
		float w = newCenter4.w;
		newCenter /= w;

		glm::mat3 rotation = glm::mat3(glm::toQuat(trans));

		for (int i = 0; i < 3; i++) {
		for (int a = 0; a < 3; a++) {
		rotation[i][a] = glm::abs(rotation[i][a]);
		}
		}

		glm::vec3 vect1 = rotation * glm::vec3(_xExtent, _yExtent, _zExtent);


		float xe = glm::abs(vect1.x);
		float ye = glm::abs(vect1.y);
		float ze = glm::abs(vect1.z);
		return new BoundingBox(newCenter, xe, ye, ze);*/
	}

	void BoundingBox::checkMinMax(glm::vec3 &min, glm::vec3 &max, glm::vec3 & point) {
		if (point.x < min.x) {
			min.x = point.x;
		}
		if (point.x > max.x) {
			max.x = point.x;
		}
		if (point.y < min.y) {
			min.y = point.y;
		}
		if (point.y > max.y) {
			max.y = point.y;
		}
		if (point.z < min.z) {
			min.z = point.z;
		}
		if (point.z > max.z) {
			max.z = point.z;
		}
	}

	Side BoundingBox::whichSide(Plane& plane) {
		float radius = glm::abs(_xExtent * plane.getNormal().x) + glm::abs(_yExtent * plane.getNormal().y) + glm::abs(_zExtent * plane.getNormal().z);

		float distance = plane.pseudoDistance(_center);

		//changed to < and > to prevent floating point precision problems
		if (distance < -radius) {
			return Side::Negative;
		} else if (distance > radius) {
			return Side::Positive;
		} else {
			return Side::None;
		}
	}

	glm::vec3 & BoundingBox::getCenter() {
		return _center;
	}

	void BoundingBox::setMinMax(glm::vec3 min, glm::vec3 max) {
		_center = max;
		_center += min;
		_center *= 0.5f;
		_xExtent = glm::abs(max.x - _center.x);
		_yExtent = glm::abs(max.y - _center.y);
		_zExtent = glm::abs(max.z - _center.z);
	}

	glm::vec3 BoundingBox::getMin() {
		return _center - glm::vec3(_xExtent, _yExtent, _zExtent);
	}

	glm::vec3 BoundingBox::getMax() {
		return _center + glm::vec3(_xExtent, _yExtent, _zExtent);
	}

	glm::vec3 BoundingBox::getExtent() {
		return glm::vec3(_xExtent, _yExtent, _zExtent);
	}

	float BoundingBox::getXExtent() {
		return _xExtent;
	}

	float BoundingBox::getYExtent() {
		return _yExtent;
	}

	float BoundingBox::getZExtent() {
		return _zExtent;
	}

	float BoundingBox::getMinX() {
		return getMin().x;
	}

	float BoundingBox::getMinY() {
		return getMin().y;
	}

	float BoundingBox::getMinZ() {
		return getMin().z;
	}

	float BoundingBox::getMaxX() {
		return getMax().x;
	}

	float BoundingBox::getMaxY() {
		return getMax().y;
	}

	float BoundingBox::getMaxZ() {
		return getMax().z;
	}

	std::string BoundingBox::toString() {
		return std::string("Center(x=") + std::string(std::to_string(_center.x)) + std::string(",y=") + std::string(std::to_string(_center.y)) + std::string(",z=") + std::string(std::to_string(_center.z)) + std::string("),xe=") + std::string(std::to_string(_xExtent)) + std::string(",ye=") + std::string(std::to_string(_yExtent)) + std::string(",ze=") + std::string(std::to_string(_zExtent));
	}

	bool BoundingBox::clip(float denom, float numer, float t[]) {
		if (denom > 0.0f) {
			float newT = numer / denom;
			if (newT > t[1]) {
				return false;
			}
			if (newT > t[0]) {
				t[0] = newT;
			}
			return true;
		} else if (denom < 0.0f) {
			float newT = numer / denom;
			if (newT < t[0]) {
				return false;
			}
			if (newT < t[1]) {
				t[1] = newT;
			}
			return true;
		} else {
			return numer <= 0.0f;
		}
	}
}
