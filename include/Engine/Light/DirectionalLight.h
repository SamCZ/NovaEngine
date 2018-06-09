#ifndef DIRECTIONAL_LIGHT_H
#define DIRECTIONAL_LIGHT_H

#include "Engine/Light/Light.h"
#include "glm/glm.hpp"

namespace NovaEngine {
	class DirectionalLight : public Light {
	private:
		glm::vec3 _direction;
	public:
		DirectionalLight();
		DirectionalLight(const ColorRGBA& color, glm::vec3 dir);

		void setDirection(const glm::vec3& dir);
		glm::vec3& getDirection();

		void setPosition(const glm::vec3& pos);
		glm::vec3& getPosition();

		void setRotation(float x, float y, float z) override;

		void updateRotation();

		LightType getType() override;
	};
}

#endif // !DIRECTIONAL_LIGHT_H