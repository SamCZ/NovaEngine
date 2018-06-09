#ifndef COLLIDABLE_H
#define COLLIDABLE_H

#include <vector>
#include "glm/glm.hpp"

namespace NovaEngine {
	class GameObject;

	struct CollisionResult {
		bool isNull = true;
		GameObject* obj;
		glm::vec3 contactPoint;
		glm::vec3 contactNormal;
		float distance;
		int triangleIndex;
	};

	class CollisionResults {
	public:
		CollisionResults();
		void clear();
		void addCollision(CollisionResult result);
		int size();
		CollisionResult getClosestCollision();
		CollisionResult getFarthestCollision();
		CollisionResult getCollision(int index);
		CollisionResult getCollisonDirect(int index);
	private:
		std::vector<CollisionResult> _results;
		bool _sorted = false;
	};
}

#endif // !COLLIDABLE_H
