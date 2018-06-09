#ifndef AMBIENT_LIGHT_H
#define AMBIENT_LIGHT_H

#include "Engine/Light/Light.h"

namespace NovaEngine{
	class AmbientLight : public Light {
	public:
		AmbientLight();
		AmbientLight(const ColorRGBA& color);

		LightType getType() override;
	};
}

#endif // !AMBIENT_LIGHT_H