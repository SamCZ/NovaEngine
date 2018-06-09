#include "Engine/Light/AmbientLight.h"

namespace NovaEngine {
	AmbientLight::AmbientLight() : Light() {
		setName("Ambient Light");
	}

	AmbientLight::AmbientLight(const ColorRGBA& color) : Light() {
		setColor(color);
	}

	LightType AmbientLight::getType() {
		return LightType::Ambient;
	}
}