#ifndef BIH_TREE_H
#define BIH_TREE_H

#include "glm/glm.hpp"
#include "glm/geometric.hpp"
#include "Engine/Collisions/Ray.h"
#include "Engine/Collisions/BoundingBox.h"
#include "Engine/Collisions/Collidable.h"

namespace NovaEngine {
	class Mesh;
	class BIHNode;

	class BIHTree {
	public:
		BIHTree(Mesh* mesh, int maxTrisPerNode);
		BIHTree(Mesh* mesh);
		~BIHTree();

		void construct();

		void getTriangle(int index, glm::vec3 &v1, glm::vec3 &v2, glm::vec3 &v3);
		int getTriangleIndex(int triIndex);
		int collideWithRay(Ray r, glm::mat4 worldMatrix, BoundingBox* worldBound, CollisionResults &results);
	private:
		const int MAX_TREE_DEPTH = 100;
		const int MAX_TRIS_PER_NODE = 21;
		Mesh* mesh;
		BIHNode* root;
		int maxTrisPerNode;
		int numTris;

		int numPointData;
		int numTriIndices;
		int numBihSwapTmp;

		float* pointData;
		int* triIndices;
		float* bihSwapTmp;

		void initTriList(Mesh* mesh);
		BoundingBox createBox(int l, int r);
		int sortTriangles(int l, int r, float split, int axis);
		void setMinMax(BoundingBox bbox, bool doMin, int axis, float value);
		float getMinMax(BoundingBox bbox, bool doMin, int axis);
		BIHNode* createNode(int l, int r, BoundingBox nodeBbox, int depth);
		void arraycopy(float* src, int srcPos, float* dest, int destPos, int length);
		void swapTriangles(int index1, int index2);

		CollisionResults _bboxTempRes;
	};

	class BIHNode {
	public:
		int leftIndex;
		int rightIndex;
		BIHNode* left;
		BIHNode* right;
		float leftPlane;
		float rightPlane;
		int axis;

		BIHNode(int l, int r);
		BIHNode(int axis);
		~BIHNode();

		BIHNode* getLeftChild();
		void setLeftChild(BIHNode* left);
		float getLeftPlane();
		void setLeftPlane(float leftPlane);
		BIHNode* getRightChild();
		void setRightChild(BIHNode* right);
		float getRightPlane();
		void setRightPlane(float rightPlane);

		int intersectWhere(Ray r, glm::mat4 worldMatrix, BIHTree* tree, float sceneMin, float sceneMax, CollisionResults &results);
	};

	class BIHStackData {
	public:
		BIHNode* node;
		float min, max;

		BIHStackData(BIHNode* node, float min, float max);
	};
}

#endif // !BIH_TREE_H
