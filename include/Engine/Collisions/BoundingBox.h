#ifndef BOUNDING_BOX_H
#define BOUNDING_BOX_H

#include "glm/glm.hpp"
#include "Engine/Render/Mesh/Mesh.h"
#include <string>
#include <vector>

namespace NovaEngine {
	class CollisionResults;
	class Ray;

	enum class Side;
	class Plane;

	class BoundingBox {
	public:
		int checkSide;

		BoundingBox();
		~BoundingBox();
		BoundingBox(BoundingBox* box);
		BoundingBox(glm::vec3 min, glm::vec3 max);
		BoundingBox(glm::vec3 center, float xExtent, float yExtent, float zExtent);
		void computeFromMesh(Mesh* mesh);
		void computeFromPoints(std::vector<glm::vec3> &points, glm::mat4* transform = nullptr);
		void computeFromPoints(float* points, int size, glm::mat4* transform = nullptr);

		int collideWithRay(Ray ray, CollisionResults& results);
		BoundingBox* transform(glm::vec3 location, glm::vec3 rotation, glm::vec3 scale);

		static void checkMinMax(glm::vec3 &min, glm::vec3 &max, glm::vec3 &point);

		Side whichSide(Plane& plane);

		glm::vec3& getCenter();

		void setMinMax(glm::vec3 min, glm::vec3 max);

		glm::vec3 getMin();
		glm::vec3 getMax();

		glm::vec3 getExtent();

		float getXExtent();
		float getYExtent();
		float getZExtent();

		float getMinX();
		float getMinY();
		float getMinZ();

		float getMaxX();
		float getMaxY();
		float getMaxZ();

		std::string toString();
	private:
		glm::vec3 _center;
		float _xExtent;
		float _yExtent;
		float _zExtent;

		float _clipTemp[3];

		bool clip(float denom, float numer, float t[]);
	};
}

#endif // !BOUNDING_BOX_H
