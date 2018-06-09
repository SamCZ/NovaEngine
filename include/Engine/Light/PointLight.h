#ifndef POINT_LIGHT_H
#define POINT_LIGHT_H

#include "Engine/Light/Light.h"
#include "glm/glm.hpp"

namespace NovaEngine {
	class TextureCubeMap;

	class PointLight : public Light {
	private:
		float _radius;
		float _invRadius;
		TextureCubeMap* _depthMap;
	public:
		PointLight();
		PointLight(const glm::vec3& pos, const ColorRGBA& color, float radius);

		void setRadius(float radius);
		float getRadius() const;

		float getInvRadius() const;

		void setPosition(const glm::vec3& pos);
		glm::vec3& getPosition();

		void setDepthMap(TextureCubeMap* map);
		TextureCubeMap* getDepthMap();
		bool hasDepthMap();

		LightType getType() override;
	};
}

#endif // !POINT_LIGHT_H