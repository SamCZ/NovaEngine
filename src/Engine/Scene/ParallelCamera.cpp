#include "Engine/Scene/ParallelCamera.h"

namespace NovaEngine {
	ParallelCamera::ParallelCamera(int width, int height) : Camera(width, height) {
		_frustumSize = 100;
		_scaleX = 1.0f;
		_scaleY = 1.0f;
		update();
	}
	void ParallelCamera::update() {
		float aspect = (float) this->getWidth() / this->getHeight();
		this->_projectionMatrix = glm::ortho((-aspect * _frustumSize) * _scaleX, (aspect * _frustumSize) * _scaleX, -_frustumSize * _scaleY, _frustumSize * _scaleY, -100.0f, 100.0f);
		Camera::update();
	}

	void ParallelCamera::setFrustumSize(float frustumSize) {
		_frustumSize = frustumSize;
	}

	void ParallelCamera::setScale(float x, float y) {
		_scaleX = x;
		_scaleY = y;
	}

	float ParallelCamera::getFrustumSize() {
		return _frustumSize;
	}

	float ParallelCamera::getScaleX() {
		return _scaleX;
	}

	float ParallelCamera::getScaleY() {
		return _scaleY;
	}
}