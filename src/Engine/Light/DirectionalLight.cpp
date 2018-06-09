#include "Engine/Light/DirectionalLight.h"
#include "glm/glm.hpp"
#include <glm/gtx/transform.hpp>

namespace NovaEngine {
	DirectionalLight::DirectionalLight() : Light() {
		setName("Directional Light");
	}

	DirectionalLight::DirectionalLight(const ColorRGBA& color, glm::vec3 dir) : DirectionalLight() {
		setColor(color);
		setDirection(dir);
	}

	void DirectionalLight::setDirection(const glm::vec3& dir) {
		_direction = dir;
	}

	glm::vec3 & DirectionalLight::getDirection() {
		return _direction;
	}

	void DirectionalLight::setPosition(const glm::vec3& pos) {
		setLocation(pos);
	}

	glm::vec3 & DirectionalLight::getPosition() {
		return getLocation();
	}

	void DirectionalLight::setRotation(float x, float y, float z) {
		Spatial::setRotation(x, y, z);
		updateRotation();
	}

	void DirectionalLight::updateRotation() {
		glm::vec3 rollVector = glm::vec3(0, 0, 1);
		glm::mat4 camera = glm::rotate(glm::mat4(), glm::radians(_rotation.z), rollVector);
		glm::vec3 upVector = glm::vec3(0, 1, 0);
		camera = glm::rotate(camera, glm::radians(_rotation.y), upVector);
		glm::vec3 pitchVector = glm::vec3(1, 0, 0);
		camera = glm::rotate(camera, glm::radians(_rotation.x), pitchVector);

		glm::vec3 pos = glm::vec3(1, 0, 0);
		glm::vec4 posTrans = camera * glm::vec4(pos, 1.0);
		_direction = glm::normalize(posTrans);
	}

	LightType DirectionalLight::getType() {
		return LightType::Directional;
	}
}