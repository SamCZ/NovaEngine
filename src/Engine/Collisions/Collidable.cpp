#include "Engine/Collisions/Collidable.h"
#include "Engine/Utils/Utils.h"

namespace NovaEngine {
	int compareCollisionResults(const CollisionResult left, const CollisionResult right) {
		return compareFloat(left.distance, right.distance);
	}

	CollisionResults::CollisionResults() : _results() {
	}

	void CollisionResults::clear() {
		_results.clear();
	}

	void CollisionResults::addCollision(CollisionResult result) {
		_results.push_back(result);
		_sorted = false;
	}

	int CollisionResults::size() {
		return _results.size();
	}

	CollisionResult CollisionResults::getClosestCollision() {
		if (size() == 0) {
			return CollisionResult();
		}
		if (!_sorted) {
			mergeSort<CollisionResult>(_results, compareCollisionResults);
			_sorted = true;
		}
		return _results[0];
	}

	CollisionResult CollisionResults::getFarthestCollision() {
		if (size() == 0) {
			return CollisionResult();
		}
		if (!_sorted) {
			mergeSort<CollisionResult>(_results, compareCollisionResults);
			_sorted = true;
		}
		return _results[size() - 1];
	}

	CollisionResult CollisionResults::getCollision(int index) {
		if (size() == 0) {
			return CollisionResult();
		}
		if (!_sorted) {
			mergeSort<CollisionResult>(_results, compareCollisionResults);
			_sorted = true;
		}
		return _results[index];
	}

	CollisionResult CollisionResults::getCollisonDirect(int index) {
		if (size() == 0) {
			return CollisionResult();
		}
		return _results[index];
	}
}
