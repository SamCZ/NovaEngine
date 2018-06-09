#include "Engine/Collisions/BIHTree.h"
#include <limits>
#include <vector>
#include <algorithm>
#include <iostream>
#include "glm/gtx/matrix_decompose.hpp"
#include "glm/gtx/quaternion.hpp"
#include "Engine/Render/Mesh/Mesh.h"
#include "Engine/Scene/GameObject.h"
#include "Engine/Render/Mesh/Triangle.h"
#include "Engine/Utils/Buffers.h"

namespace NovaEngine {
	BIHNode::BIHNode(int l, int r) {
		leftIndex = l;
		rightIndex = r;
		axis = 3;
	}

	BIHNode::BIHNode(int axis) {
		this->axis = axis;
	}

	BIHNode::~BIHNode() {
		delete left;
		delete right;
	}

	BIHNode * BIHNode::getLeftChild() {
		return left;
	}

	void BIHNode::setLeftChild(BIHNode * left) {
		this->left = left;
	}

	float BIHNode::getLeftPlane() {
		return leftPlane;
	}

	void BIHNode::setLeftPlane(float leftPlane) {
		this->leftPlane = leftPlane;
	}

	BIHNode * BIHNode::getRightChild() {
		return right;
	}

	void BIHNode::setRightChild(BIHNode * right) {
		this->right = right;
	}

	float BIHNode::getRightPlane() {
		return rightPlane;
	}

	void BIHNode::setRightPlane(float rightPlane) {
		this->rightPlane = rightPlane;
	}

	struct IntersectComparator {

	} iCompare;

	glm::quat getRotationFromMatrix(glm::mat4 mat) {
		glm::vec3 scale;
		glm::quat rotation;
		glm::vec3 translation;
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(mat, scale, rotation, translation, skew, perspective);
		return glm::conjugate(rotation);
	}

	glm::vec3 multNormal(glm::mat4 mat, glm::vec3 vec) {
		const float *pSource = (const float*)glm::value_ptr(mat);
		float vx = vec.x, vy = vec.y, vz = vec.z;

		glm::vec3 store = glm::vec3();

		/*
		store.x = m00 * vx + m01 * vy + m02 * vz;
		store.y = m10 * vx + m11 * vy + m12 * vz;
		store.z = m20 * vx + m21 * vy + m22 * vz;*/

		/*
		store.x = pSource[0] * vx + pSource[1] * vy + pSource[2] * vz;//pSource[3]
		store.y = pSource[4] * vx + pSource[5] * vy + pSource[6] * vz;//pSource[7]
		store.z = pSource[8] * vx + pSource[9] * vy + pSource[10] * vz;//pSource[11]
		*/
		/*
		store.x = pSource[0] * vx + pSource[4] * vy + pSource[8] * vz;
		store.y = pSource[1] * vx + pSource[5] * vy + pSource[9] * vz;
		store.z = pSource[2] * vx + pSource[6] * vy + pSource[10] * vz;
		*/

		// m00 m01 m02 m03
		// m10 m11 m12 m13
		// m20 m21 m22 m23
		// m30 m31 m32 m33

		// m00 m10 m20 m30
		// m01 m11 m21 m31
		// m02 m12 m22 m32
		// m03 m13 m32 m33

		return store;
	}

	glm::vec3 mult(glm::mat4 mat, glm::vec3 vec) {
		const float *pSource = (const float*)glm::value_ptr(mat);
		glm::vec3 store = glm::vec3();
		float vx = vec.x, vy = vec.y, vz = vec.z;
		/*
		store.x = pSource[0] * vx + pSource[1] * vy + pSource[2] * vz + pSource[3];
		store.y = pSource[4] * vx + pSource[5] * vy + pSource[6] * vz + pSource[7];
		store.z = pSource[8] * vx + pSource[9] * vy + pSource[10] * vz + pSource[11];
		*/

		store.x = pSource[0] * vx + pSource[4] * vy + pSource[8] * vz + pSource[12];
		store.y = pSource[1] * vx + pSource[5] * vy + pSource[9] * vz + pSource[13];
		store.z = pSource[2] * vx + pSource[6] * vy + pSource[10] * vz + pSource[14];

		return store;
	}

	int BIHNode::intersectWhere(Ray r, glm::mat4 worldMatrix, BIHTree * tree, float sceneMin, float sceneMax, CollisionResults& results) {
		std::vector<BIHStackData> stack;

		glm::vec3 o = r.origin;
		glm::vec3 d = r.direction;

		glm::mat4 inv = glm::inverse(worldMatrix);
		glm::quat rotation = glm::toQuat(inv);

		r.origin = mult(inv, r.origin);
		//r.direction = glm::conjugate(rotation) * r.direction * rotation;
		r.direction = glm::mat3(inv) * r.direction;

		float origins[3] = { 
			r.origin.x,
			r.origin.y,
			r.origin.z 
		};

		float invDirections[3] = { 
			1.0f / r.direction.x,
			1.0f / r.direction.y,
			1.0f / r.direction.z
		};

		r.direction = glm::normalize(r.direction);

		glm::vec3 v1 = glm::vec3();
		glm::vec3 v2 = glm::vec3();
		glm::vec3 v3 = glm::vec3();
		int cols = 0;

		stack.push_back(BIHStackData(this, sceneMin, sceneMax));

		stackloop:
		while (stack.size() > 0) {
			BIHStackData data = stack[stack.size() - 1];
			stack.erase(stack.begin() + stack.size() - 1);
			BIHNode* node = data.node;
			float tMin = data.min, tMax = data.max;
			if (tMax < tMin) {
				continue;
			}

			leafloop:
			while (node->axis != 3) {
				int a = node->axis;

				// find the origin and direction value for the given axis
				float origin = origins[a];
				float invDirection = invDirections[a];

				float tNearSplit, tFarSplit;
				BIHNode* nearNode;
				BIHNode* farNode;

				tNearSplit = (node->leftPlane - origin) * invDirection;
				tFarSplit = (node->rightPlane - origin) * invDirection;
				nearNode = node->left;
				farNode = node->right;

				if (invDirection < 0) {
					float tmpSplit = tNearSplit;
					tNearSplit = tFarSplit;
					tFarSplit = tmpSplit;

					BIHNode* tmpNode = nearNode;
					nearNode = farNode;
					farNode = tmpNode;
				}

				if (tMin > tNearSplit && tMax < tFarSplit) {
					goto stackloop;
				}

				if (tMin > tNearSplit) {
					tMin = glm::max(tMin, tFarSplit);
					node = farNode;
				} else if (tMax < tFarSplit) {
					tMax = glm::min(tMax, tNearSplit);
					node = nearNode;
				} else {
					stack.push_back(BIHStackData(farNode, glm::max(tMin, tFarSplit), tMax));
					tMax = glm::min(tMax, tNearSplit);
					node = nearNode;
				}
			}

			// a leaf
			for (int i = node->leftIndex; i <= node->rightIndex; i++) {
				tree->getTriangle(i, v1, v2, v3);
				
				float t = r.intersects(v1, v2, v3);
				if (!std::isinf(t)) {
					if (true) {
						v1 = worldMatrix * glm::vec4(v1, 1.0f);
						v2 = worldMatrix * glm::vec4(v2, 1.0f);
						v3 = worldMatrix * glm::vec4(v3, 1.0f);
						float t_world = (new Ray(o, d))->intersects(v1, v2, v3);
						if (std::isinf(t_world)) {
							continue;
						}
						t = t_world;
					}
					glm::vec3 contactNormal = Triangle::computeTriangleNormal(v1, v2, v3);
					glm::vec3 contactPoint = (glm::vec3(d) * t) + o;

					CollisionResult res;
					res.isNull = false;
					res.contactNormal = contactNormal;
					res.contactPoint = contactPoint;
					res.distance = glm::distance(o, contactPoint);
					res.triangleIndex = tree->getTriangleIndex(i);
					results.addCollision(res);
					cols++;
				}
			}
		}
		stack.clear();
		r.origin = o;
		r.direction = d;
		return cols;
	}

	BIHStackData::BIHStackData(BIHNode * node, float min, float max) {
		this->node = node;
		this->min = min;
		this->max = max;
	}

	BIHTree::BIHTree(Mesh * mesh, int maxTrisPerNode) {
		this->mesh = mesh;
		this->maxTrisPerNode = maxTrisPerNode;
		this->numBihSwapTmp = 9;
		this->bihSwapTmp = new float[9];
		this->initTriList(mesh);
	}

	BIHTree::BIHTree(Mesh * mesh) : BIHTree(mesh, MAX_TRIS_PER_NODE) {
	}

	BIHTree::~BIHTree() {
		delete[] pointData;
		delete[] triIndices;
		delete[] bihSwapTmp;
		delete root;
	}

	void BIHTree::construct() {
		BoundingBox sceneBbox = createBox(0, this->numTris - 1);
		this->root = createNode(0, this->numTris - 1, sceneBbox, 0);
	}

	void BIHTree::initTriList(Mesh* mesh) {
		VertexBuffer* indexVertexBuffer = mesh->getVertexBuffer(MeshBuffer::Type::Index);
		VertexBuffer* posVertexBuffer = mesh->getVertexBuffer(MeshBuffer::Type::Position);

		if (indexVertexBuffer == nullptr) {
			std::cout << "NO INDEX" << std::endl;
			return;
		}

		IntBuffer* indices = static_cast<IntBuffer*>(indexVertexBuffer->getBuffer());
		FloatBuffer* vertices = static_cast<FloatBuffer*>(posVertexBuffer->getBuffer());

		if (indices->size() == 0) {
			return;
		}

		this->numTris = indices->size() / 3;

		this->numPointData = numTris * 3 * 3;
		this->pointData = new float[this->numPointData];
		int p = 0;
		for (int i = 0; i < numTris; i++) {
			int i1 = indices->get(i * 3 + 0);
			int i2 = indices->get(i * 3 + 1);
			int i3 = indices->get(i * 3 + 2);
			glm::vec3 point = glm::vec3(vertices->get(i1 * 3 + 0), vertices->get(i1 * 3 + 1), vertices->get(i1 * 3 + 2));
			pointData[p++] = point.x;
			pointData[p++] = point.y;
			pointData[p++] = point.z;

			point = glm::vec3(vertices->get(i2 * 3 + 0), vertices->get(i2 * 3 + 1), vertices->get(i2 * 3 + 2));
			pointData[p++] = point.x;
			pointData[p++] = point.y;
			pointData[p++] = point.z;

			point = glm::vec3(vertices->get(i3 * 3 + 0), vertices->get(i3 * 3 + 1), vertices->get(i3 * 3 + 2));
			pointData[p++] = point.x;
			pointData[p++] = point.y;
			pointData[p++] = point.z;
		}

		this->numTriIndices = numTris;
		this->triIndices = new int[numTris];
		for (int i = 0; i < numTris; i++) {
			this->triIndices[i] = i;
		}

		/*VertexBuffer* indexBuffer = mesh->getBuffer(BufferType::Index);
		VertexBuffer* vertexBuffer = mesh->getBuffer(BufferType::Position);

		if (indexBuffer == nullptr) {
			std::cout << "NO INDEX" << std::endl;
			return;
		}

		if (indexBuffer->dataInt.size() == 0) {
			return;
		}

		this->numTris = indexBuffer->dataInt.size() / 3;

		this->numPointData = numTris * 3 * 3;
		this->pointData = new float[this->numPointData];
		int p = 0;
		for (int i = 0; i < numTris; i++) {
			glm::vec3 point = vertexBuffer->dataVec3[indexBuffer->dataInt[i * 3 + 0]];
			pointData[p++] = point.x;
			pointData[p++] = point.y;
			pointData[p++] = point.z;

			point = vertexBuffer->dataVec3[indexBuffer->dataInt[i * 3 + 1]];
			pointData[p++] = point.x;
			pointData[p++] = point.y;
			pointData[p++] = point.z;

			point = vertexBuffer->dataVec3[indexBuffer->dataInt[i * 3 + 2]];
			pointData[p++] = point.x;
			pointData[p++] = point.y;
			pointData[p++] = point.z;
		}

		this->numTriIndices = numTris;
		this->triIndices = new int[numTris];
		for (int i = 0; i < numTris; i++) {
			this->triIndices[i] = i;
		}*/
	}

	BoundingBox BIHTree::createBox(int l, int r) {
		float fmax = std::numeric_limits<float>::max();
		float fmin = std::numeric_limits<float>::max();

		glm::vec3 max = glm::vec3(fmin, fmin, fmin);
		glm::vec3 min = glm::vec3(fmax, fmax, fmax);

		glm::vec3 v1 = glm::vec3();
		glm::vec3 v2 = glm::vec3();
		glm::vec3 v3 = glm::vec3();

		for (int i = l; i <= r; i++) {
			getTriangle(i, v1, v2, v3);
			BoundingBox::checkMinMax(min, max, v1);
			BoundingBox::checkMinMax(min, max, v2);
			BoundingBox::checkMinMax(min, max, v3);
		}

		return BoundingBox(min, max);
	}

	int BIHTree::getTriangleIndex(int triIndex) {
		return this->triIndices[triIndex];
	}

	int BIHTree::sortTriangles(int l, int r, float split, int axis) {
		int pivot = l;
		int j = r;

		glm::vec3 v1 = glm::vec3();
		glm::vec3 v2 = glm::vec3();
		glm::vec3 v3 = glm::vec3();

		while (pivot <= j) {
			getTriangle(pivot, v1, v2, v3);
			v1 += v2;
			v1 += v3;
			v1 *= 1.0f / 3.0f;
			if (v1[axis] > split) {
				swapTriangles(pivot, j);
				--j;
			} else {
				++pivot;
			}
		}
		pivot = (pivot == l && j < pivot) ? j : pivot;

		return pivot;
	}

	void BIHTree::setMinMax(BoundingBox bbox, bool doMin, int axis, float value) {
		glm::vec3 min = bbox.getMin();
		glm::vec3 max = bbox.getMax();

		if (doMin) {
			min[axis] = value;
		} else {
			max[axis] = value;
		}
		bbox.setMinMax(min, max);
	}

	float BIHTree::getMinMax(BoundingBox bbox, bool doMin, int axis) {
		if (doMin) {
			return bbox.getMin()[axis];
		} else {
			return bbox.getMax()[axis];
		}
	}

	BIHNode * BIHTree::createNode(int l, int r, BoundingBox nodeBbox, int depth) {
		if ((r - l) < maxTrisPerNode || depth > MAX_TREE_DEPTH) {
			return new BIHNode(l, r);
		}
		BoundingBox currentBox = createBox(l, r);

		glm::vec3 exteriorExt = nodeBbox.getExtent();
		glm::vec3 interiorExt = currentBox.getExtent();

		exteriorExt -= interiorExt;

		int axis = 0;
		if (exteriorExt.x > exteriorExt.y) {
			if (exteriorExt.x > exteriorExt.z) {
				axis = 0;
			} else {
				axis = 2;
			}
		} else {
			if (exteriorExt.y > exteriorExt.z) {
				axis = 1;
			} else {
				axis = 2;
			}
		}
		if (exteriorExt == glm::vec3()) {
			axis = 0;
		}

		float split = currentBox.getCenter()[axis];
		int pivot = sortTriangles(l, r, split, axis);
		if (pivot == l || pivot == r) {
			pivot = (r + l) / 2;
		}

		if (pivot < l) {
			//Only right
			BoundingBox rbbox = BoundingBox(currentBox);
			setMinMax(rbbox, true, axis, split);
			return createNode(l, r, rbbox, depth + 1);
		} else if (pivot > r) {
			//Only left
			BoundingBox lbbox = BoundingBox(currentBox);
			setMinMax(lbbox, false, axis, split);
			return createNode(l, r, lbbox, depth + 1);
		} else {
			//Build the node
			BIHNode* node = new BIHNode(axis);

			//Left child
			BoundingBox lbbox = BoundingBox(currentBox);
			setMinMax(lbbox, false, axis, split);

			//The left node right border is the plane most right
			node->setLeftPlane(getMinMax(createBox(l, glm::max(l, pivot - 1)), false, axis));
			node->setLeftChild(createNode(l, glm::max(l, pivot - 1), lbbox, depth + 1)); //Recursive call

																						 //Right Child
			BoundingBox rbbox = BoundingBox(currentBox);
			setMinMax(rbbox, true, axis, split);
			//The right node left border is the plane most left
			node->setRightPlane(getMinMax(createBox(pivot, r), true, axis));
			node->setRightChild(createNode(pivot, r, rbbox, depth + 1)); //Recursive call

			return node;
		}

		return nullptr;
	}

	void BIHTree::getTriangle(int index, glm::vec3 & v1, glm::vec3 & v2, glm::vec3 & v3) {
		int pointIndex = index * 9;

		v1.x = pointData[pointIndex++];
		v1.y = pointData[pointIndex++];
		v1.z = pointData[pointIndex++];

		v2.x = pointData[pointIndex++];
		v2.y = pointData[pointIndex++];
		v2.z = pointData[pointIndex++];

		v3.x = pointData[pointIndex++];
		v3.y = pointData[pointIndex++];
		v3.z = pointData[pointIndex++];
	}

	void BIHTree::arraycopy(float * src, int srcPos, float * dest, int destPos, int length) {
		while (length > 0) {
			dest[destPos] = src[srcPos];
			srcPos++;
			destPos++;
			length--;
		}
	}

	void BIHTree::swapTriangles(int index1, int index2) {
		int p1 = index1 * 9;
		int p2 = index2 * 9;

		arraycopy(pointData, p1, bihSwapTmp, 0, 9);
		// copy p2 to p1
		arraycopy(pointData, p2, pointData, p1, 9);
		// copy tmp to p2
		arraycopy(bihSwapTmp, 0, pointData, p2, 9);

		int tmp2 = triIndices[index1];
		triIndices[index1] = triIndices[index2];
		triIndices[index2] = tmp2;
	}

	int BIHTree::collideWithRay(Ray r, glm::mat4 worldMatrix, BoundingBox * worldBound, CollisionResults& results) {
		_bboxTempRes.clear();
		worldBound->collideWithRay(r, _bboxTempRes);
		if (_bboxTempRes.size() > 0) {
			float tMin = _bboxTempRes.getClosestCollision().distance;
			float tMax = _bboxTempRes.getFarthestCollision().distance;

			if (tMax <= 0) {
				tMax = std::numeric_limits<float>::infinity();
			} else if (tMin == tMax) {
				tMin = 0;
			}

			if (tMin <= 0) {
				tMin = 0;
			}

			if (r.limit < std::numeric_limits<float>::infinity()) {
				tMax = glm::min(tMax, r.limit);
				if (tMin > tMax) {
					return 0;
				}
			}

			return this->root->intersectWhere(r, worldMatrix, this, tMin, tMax, results);
		}
		return 0;
	}
}
