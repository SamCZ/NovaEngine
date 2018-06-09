#ifndef LIGHT_H
#define LIGHT_H

#include "Engine/Utils/ColorRGBA.h"
#include "Engine/Scene/Spatial.h"

namespace NovaEngine {
	enum class LightType {
		Ambient,
		Directional,
		Point,
		Spot
	};

	class Light : public Spatial {
	protected:
		ColorRGBA _color;
		bool _enabled;
		float _intensity;
	public:
		Light();

		void setColor(const ColorRGBA& color);
		ColorRGBA& getColor();

		void setEnabled(bool enabled);
		bool isEnabled() const;

		void setIntensity(float intensity);
		float& getIntensity();

		virtual LightType getType() = 0;
	};
}

#endif // !LIGHT_H