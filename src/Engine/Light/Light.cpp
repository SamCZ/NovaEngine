#include "Engine/Light/Light.h"

namespace NovaEngine {
	Light::Light() : Spatial("Light"), _color(makeRGB(255, 244, 214)), _intensity(1.0f) {}

	void Light::setColor(const ColorRGBA& color) {
		_color = color;
	}

	void Light::setEnabled(bool enabled) {
		_enabled = enabled;
	}

	bool Light::isEnabled() const {
		return _enabled;
	}

	void Light::setIntensity(float intensity) {
		_intensity = intensity;
	}

	float& Light::getIntensity() {
		return _intensity;
	}

	ColorRGBA& Light::getColor() {
		return _color;
	}
}