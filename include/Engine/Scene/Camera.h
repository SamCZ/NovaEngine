#ifndef CAMERA_H
#define CAMERA_H

#include "glm/glm.hpp"
#include <glm/gtx/transform.hpp>
#include "Engine/Collisions/Plane.h"
#include "Engine/Collisions/BoundingBox.h"
#include "Engine/Collisions/Ray.h"
#include "Engine/Scene/Spatial.h"

namespace NovaEngine {
	enum class FrustumIntersect {
		Outside,
		Inside,
		Intersects
	};

	std::string frustumEnumToString(const FrustumIntersect& inters);

	class Camera : public Spatial {
	public:
		Camera(int width, int height);

		void setProjectionMatrix(glm::mat4 m);
		void setViewMatrix(glm::mat4 m);

		glm::mat4& getProjectionMatrix();
		glm::mat4& getViewMatrix();
		glm::mat4& getProjectionViewMatrix();

		void setFromFrustum(float left, float right, float bottom, float top, float zNear, float zFar);
		void setFrustumPerspective(float fovY, float aspect, float zNear, float zFar);
		void resize(int width, int height);

		void lookAt(glm::vec3 toPos, glm::vec3 eye);
		void lookAtDir(glm::vec3 dir);

		glm::vec3 getWorldPosition(float x, float y, float projectionZPos) const;
		glm::vec3 getScreenCoordinates(glm::vec3 pos);

		virtual void update();

		int getWidth() const;
		int getHeight() const;

		float getFrustumLeft();
		float getFrustumRight();
		float getFrustumBottom();
		float getFrustumTop();
		float getFrustumNear();
		float getFrustumFar();

		glm::vec3 getLeft();
		glm::vec3 getDirection();
		glm::vec3 getUp();

		FrustumIntersect contains(BoundingBox* bound);

		Ray getRay(int x, int y);
	protected:
		static const int ORTHOGRAPHIC_MODE = 0;
		static const int PERSPECTIVE_MODE = 1;
		
		int _planeState;

		float frustumLeft;
		float frustumRight;
		float frustumBottom;
		float frustumTop;
		float frustumNear;
		float frustumFar;

		float coeffLeft[2];
		float coeffRight[2];
		float coeffBottom[2];
		float coeffTop[2];

		static constexpr int LEFT_PLANE = 0;
		static constexpr int RIGHT_PLANE = 1;
		static constexpr int BOTTOM_PLANE = 2;
		static constexpr int TOP_PLANE = 3;
		static constexpr int FAR_PLANE = 4;
		static constexpr int NEAR_PLANE = 5;

		static constexpr int FRUSTUM_PLANES = 6;
		static constexpr int MAX_WORLD_PLANES = 6;

		Plane worldPlane[MAX_WORLD_PLANES];

		int _width;
		int _height;
		glm::mat4 _projectionMatrix;
		glm::mat4 _viewMatrix;
		glm::mat4 _viewProjectionMatrix;
		glm::mat4 _looAtMatrix;
		int _mode;
		float _fovY;
		float _zNear;
		float _zFar;
		bool _is2DOrtho;
	};
}

#endif // !CAMERA_H