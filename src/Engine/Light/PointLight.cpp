#include "Engine/Light/PointLight.h"
#include "Engine/Render/TextureCubeMap.h"

namespace NovaEngine {
	PointLight::PointLight() : _depthMap(nullptr) {
		setName("Point Light");
	}

	PointLight::PointLight(const glm::vec3 & pos, const ColorRGBA & color, float radius) : PointLight() {
		setPosition(pos);
		setColor(color);
		setRadius(radius);
	}

	void PointLight::setRadius(float radius) {
		if (radius < 0) {
			radius = -radius;
		}
		_radius = radius;
		if (radius != 0) {
			_invRadius = 1.0f / radius;
		} else {
			_invRadius = 0;
		}
	}

	float PointLight::getRadius() const {
		return _radius;
	}

	float PointLight::getInvRadius() const {
		return _invRadius;
	}

	void PointLight::setPosition(const glm::vec3& pos) {
		setLocation(pos);
	}

	glm::vec3& PointLight::getPosition() {
		return getLocation();
	}

	void PointLight::setDepthMap(TextureCubeMap* map) {
		_depthMap = map;
	}

	TextureCubeMap* PointLight::getDepthMap() {
		return _depthMap;
	}

	bool PointLight::hasDepthMap() {
		return _depthMap != nullptr;
	}

	LightType PointLight::getType() {
		return LightType::Point;
	}
}