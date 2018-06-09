#include "Engine/Scene/Camera.h"
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/quaternion.hpp>

namespace NovaEngine {

	std::string frustumEnumToString(const FrustumIntersect& inters) {
		if (inters == FrustumIntersect::Inside) {
			return "Inside";
		} else if (inters == FrustumIntersect::Intersects) {
			return "Intersects";
		} else if (inters == FrustumIntersect::Outside) {
			return "Outside";
		}
		return "None";
	}

	Camera::Camera(int width, int height) : Spatial("Camera"), _projectionMatrix(), _viewMatrix(), _viewProjectionMatrix(), _looAtMatrix() {
		_width = width;
		_height = height;
		this->setFromFrustum(0.0f, (float)width, (float)height, 0.0f, -1.0f, 1.0f);
		_planeState = 0;
	}

	glm::mat4& Camera::getProjectionMatrix() {
		return _projectionMatrix;
	}

	glm::mat4& Camera::getViewMatrix() {
		return _viewMatrix;
	}

	glm::mat4& Camera::getProjectionViewMatrix() {
		return _viewProjectionMatrix;
	}

	void Camera::setFromFrustum(float left, float right, float bottom, float top, float zNear, float zFar) {
		_projectionMatrix = glm::ortho(left, right, bottom, top, zNear, zFar);
		_mode = ORTHOGRAPHIC_MODE;
		if (left == 0.0f && top == 0.0f && zNear == -1.0f && zFar == 1.0f) {
			_is2DOrtho = true;
		} else {
			_is2DOrtho = false;
		}
	}

	float Camera::getFrustumLeft() {
		return frustumLeft;
	}

	float Camera::getFrustumRight() {
		return frustumRight;
	}

	float Camera::getFrustumBottom() {
		return frustumBottom;
	}

	float Camera::getFrustumTop() {
		return frustumTop;
	}

	float Camera::getFrustumNear() {
		return frustumNear;
	}

	float Camera::getFrustumFar() {
		return frustumFar;
	}

	void Camera::setFrustumPerspective(float fovY, float aspect, float zNear, float zFar) {
		_fovY = fovY;
		_zNear = zNear;
		_zFar = zFar;
		//this->_projectionMatrix = glm::perspective(glm::radians(fovY), aspect, zNear, zFar);
		_mode = PERSPECTIVE_MODE;

		float h = glm::tan(glm::radians(fovY) * .5f) * zNear;
		float w = h * aspect;
		frustumLeft = -w;
		frustumRight = w;
		frustumBottom = -h;
		frustumTop = h;
		frustumNear = zNear;
		frustumFar = zFar;

		this->_projectionMatrix = glm::frustum(frustumLeft, frustumRight, frustumBottom, frustumTop, frustumNear, frustumFar);

		{
			float nearSquared = frustumNear * frustumNear;
			float leftSquared = frustumLeft * frustumLeft;
			float rightSquared = frustumRight * frustumRight;
			float bottomSquared = frustumBottom * frustumBottom;
			float topSquared = frustumTop * frustumTop;

			float inverseLength = 1.0f / glm::sqrt(nearSquared + leftSquared);
			coeffLeft[0] = -frustumNear * inverseLength;
			coeffLeft[1] = -frustumLeft * inverseLength;

			inverseLength = 1.0f / glm::sqrt(nearSquared + rightSquared);
			coeffRight[0] = frustumNear * inverseLength;
			coeffRight[1] = frustumRight * inverseLength;

			inverseLength = 1.0f / glm::sqrt(nearSquared + bottomSquared);
			coeffBottom[0] = frustumNear * inverseLength;
			coeffBottom[1] = -frustumBottom * inverseLength;

			inverseLength = 1.0f / glm::sqrt(nearSquared + topSquared);
			coeffTop[0] = -frustumNear * inverseLength;
			coeffTop[1] = frustumTop * inverseLength;
		}
	}

	glm::vec3 getRotationColumn(glm::mat4 mat, int i) {
		glm::quat q = glm::quat(mat);

		float norm = q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z;
		if (norm != 1.0f) {
			norm = 1.0f / glm::sqrt(norm);
		}

		float xx = q.x * q.x * norm;
		float xy = q.x * q.y * norm;
		float xz = q.x * q.z * norm;
		float xw = q.x * q.w * norm;
		float yy = q.y * q.y * norm;
		float yz = q.y * q.z * norm;
		float yw = q.y * q.w * norm;
		float zz = q.z * q.z * norm;
		float zw = q.z * q.w * norm;

		glm::vec3 store;

		switch (i) {
			case 0:
			store.x = 1 - 2 * (yy + zz);
			store.y = 2 * (xy + zw);
			store.z = 2 * (xz - yw);
			break;
			case 1:
			store.x = 2 * (xy - zw);
			store.y = 1 - 2 * (xx + zz);
			store.z = 2 * (yz + xw);
			break;
			case 2:
			store.x = 2 * (xz + yw);
			store.y = 2 * (yz - xw);
			store.z = 1 - 2 * (xx + yy);
			break;
		}
		return store;
	}

	glm::vec3 Camera::getLeft() {
		return getRotationColumn(getViewMatrix(), 0);
	}

	glm::vec3 Camera::getDirection() {
		return getRotationColumn(getViewMatrix(), 2);
	}

	glm::vec3 Camera::getUp() {
		return getRotationColumn(getViewMatrix(), 1);
	}

	FrustumIntersect Camera::contains(BoundingBox* bound) {
		if (!bound || bound == nullptr) {
			return FrustumIntersect::Inside;
		}

		for (int planeCounter = FRUSTUM_PLANES; planeCounter >= 0; planeCounter--) {
			int planeId = planeCounter;
			
			Side side = bound->whichSide(worldPlane[planeId]);

			if (side == Side::Negative) {
				//object is outside of frustum
				return FrustumIntersect::Outside;
			} else if (side == Side::Positive) {
				//object is visible on *this* plane, so mark this plane
				//so that we don't check it for sub nodes.

			}

		}

		return FrustumIntersect::Inside;
	}

	Ray Camera::getRay(int x, int y) {
		glm::vec3 click3d = getWorldPosition(x, y, 0);
		glm::vec3 dir = glm::normalize(getWorldPosition(x, y, 1) - click3d);
		return Ray(click3d, dir);
	}

	void Camera::resize(int width, int height) {
		_width = width;
		_height = height;
		if (_mode == PERSPECTIVE_MODE) {
			//setFrustumPerspective(_fovY, (float)width / (float)height, _zNear, _zFar);
		} else if (_mode == ORTHOGRAPHIC_MODE && _is2DOrtho) {
			this->setFromFrustum(0.0f, (float)width, (float)height, 0.0f, -1.0f, 1.0f);
		}
	}

	void Camera::lookAt(glm::vec3 toPos, glm::vec3 eye) {
		_looAtMatrix = glm::lookAt(eye, toPos, glm::vec3(0, 1, 0));
	}

	void Camera::lookAtDir(glm::vec3 dir) {
		_looAtMatrix = glm::lookAt(glm::vec3(), dir, glm::vec3(0, 1, 0));
	}

	glm::vec3 Camera::getWorldPosition(float x, float y, float projectionZPos) const {
		glm::mat4 inverseMat = glm::inverse(_viewProjectionMatrix);

		glm::vec3 store = glm::vec3((2.0f * x) / _width - 1.0f, (2.0f * y) / _height - 1.0f, projectionZPos * 2 - 1);
		glm::vec4 proStore = inverseMat * glm::vec4(store, 1.0f);
		store.x = proStore.x;
		store.y = proStore.y;
		store.z = proStore.z;
		store *= 1.0f / proStore.w;
		return store;
	}

	glm::vec3 Camera::getScreenCoordinates(glm::vec3 pos) {
		glm::vec4 proj = (getProjectionViewMatrix() * glm::vec4(pos, 1.0f));

		glm::vec3 store = glm::vec3(proj.x, proj.y, proj.z);
		store = store / proj.w;

		int viewPortRight = 1;
		int viewPortLeft = 0;
		int viewPortTop = 0;
		int viewPortBottom = 1;

		store.x = ((store.x + 1.0f) * (viewPortRight - viewPortLeft) / 2.0f + viewPortLeft) * getWidth();
		store.y = ((store.y + 1.0f) * (viewPortTop - viewPortBottom) / 2.0f + viewPortBottom) * getHeight();
		store.z = (store.z + 1.0f) / 2.0f;

		return store;
	}

	void Camera::update() {
		this->_viewMatrix = glm::mat4();
		// define your up vector
		glm::vec3 upVector = glm::vec3(0, 1, 0);
		// rotate around to a given bearing: yaw
		glm::mat4 camera = glm::rotate(glm::mat4(), glm::radians(_rotation.y), upVector);
		// Define the 'look up' axis, should be orthogonal to the up axis
		glm::vec3 pitchVector = glm::vec3(1, 0, 0);
		// rotate around to the required head tilt: pitch
		camera = glm::rotate(camera, glm::radians(_rotation.x), pitchVector);

		glm::vec3 rollVector = glm::vec3(0, 0, 1);
		camera = glm::rotate(camera, glm::radians(_rotation.z), rollVector);

		// now get the view matrix by taking the camera inverse
		this->_viewMatrix = _looAtMatrix * glm::inverse(camera) * glm::translate(this->_viewMatrix, glm::vec3(-_location.x, -_location.y, -_location.z));

		_viewProjectionMatrix = _projectionMatrix * _viewMatrix;

		glm::vec3 left = getLeft();
		glm::vec3 direction = getDirection();
		glm::vec3 up = getUp();

		//this->_viewMatrix = glm::lookAt(_location, direction, up);

		float dirDotLocation = glm::dot(direction, _location);

		// left plane
		glm::vec3& leftPlaneNormal = worldPlane[LEFT_PLANE].getNormal();
		leftPlaneNormal.x = left.x * coeffLeft[0];
		leftPlaneNormal.y = left.y * coeffLeft[0];
		leftPlaneNormal.z = left.z * coeffLeft[0];
		leftPlaneNormal += glm::vec3(direction.x * coeffLeft[1], direction.y * coeffLeft[1], direction.z * coeffLeft[1]);
		worldPlane[LEFT_PLANE].setConstant(glm::dot(_location, leftPlaneNormal));

		// right plane
		glm::vec3& rightPlaneNormal = worldPlane[RIGHT_PLANE].getNormal();
		rightPlaneNormal.x = left.x * coeffRight[0];
		rightPlaneNormal.y = left.y * coeffRight[0];
		rightPlaneNormal.z = left.z * coeffRight[0];
		rightPlaneNormal += glm::vec3(direction.x * coeffRight[1], direction.y * coeffRight[1], direction.z * coeffRight[1]);
		worldPlane[RIGHT_PLANE].setConstant(glm::dot(_location, rightPlaneNormal));

		// bottom plane
		glm::vec3& bottomPlaneNormal = worldPlane[BOTTOM_PLANE].getNormal();
		bottomPlaneNormal.x = up.x * coeffBottom[0];
		bottomPlaneNormal.y = up.y * coeffBottom[0];
		bottomPlaneNormal.z = up.z * coeffBottom[0];
		bottomPlaneNormal += glm::vec3(direction.x * coeffBottom[1], direction.y * coeffBottom[1], direction.z * coeffBottom[1]);
		worldPlane[BOTTOM_PLANE].setConstant(glm::dot(_location, bottomPlaneNormal));

		// top plane
		glm::vec3& topPlaneNormal = worldPlane[TOP_PLANE].getNormal();
		topPlaneNormal.x = up.x * coeffTop[0];
		topPlaneNormal.y = up.y * coeffTop[0];
		topPlaneNormal.z = up.z * coeffTop[0];
		topPlaneNormal += glm::vec3(direction.x * coeffTop[1], direction.y * coeffTop[1], direction.z * coeffTop[1]);
		worldPlane[TOP_PLANE].setConstant(glm::dot(_location, topPlaneNormal));

		// far plane
		worldPlane[FAR_PLANE].setNormal(left);
		worldPlane[FAR_PLANE].setNormal(-direction.x, -direction.y, -direction.z);
		worldPlane[FAR_PLANE].setConstant(-(dirDotLocation + frustumFar));

		// near plane
		worldPlane[NEAR_PLANE].setNormal(direction.x, direction.y, direction.z);
		worldPlane[NEAR_PLANE].setConstant(dirDotLocation + frustumNear);
	}

	int Camera::getWidth() const {
		return _width;
	}

	int Camera::getHeight() const {
		return _height;
	}

	void Camera::setProjectionMatrix(glm::mat4 m) {
		_projectionMatrix = m;
	}

	void Camera::setViewMatrix(glm::mat4 m) {
		_viewMatrix = m;
	}
}