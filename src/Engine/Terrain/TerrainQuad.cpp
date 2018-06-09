#include "Engine/Terrain/TerrainQuad.h"

#include "Engine/Utils/FastNoise.h"
#include "Engine/Utils/Buffers.h"
#include "Engine/Terrain/HeightStorage.h"
#include <iostream>

namespace NovaEngine {

	Mesh* generateTerrainQuada(HeightStorage* heightStorage, float xx, float zz, int size, float patchSize) {
		int vertexCount = size * size * 3;

		FloatBuffer* vertices = FloatBuffer::allocate(vertexCount);
		FloatBuffer* texCoords = FloatBuffer::allocate(size * size * 2);



		for (int i = 0; i < size; i++) {
			for (int j = 0; j < size; j++) {
				float x = i * patchSize;
				float z = j * patchSize;
				float y = heightStorage->getHeight((int)((xx + x) / 1), (int)((zz + z) / 1));

				//y = plains;

				/*mountainHeight += perlin.GetNoise((xx + x) * scale, (zz + z) * scale) * 2;
				//mountainHeight += myNoise.GetNoise((xx + x) * 20.0f, (zz + z) * 20.0f) * 0.025f;
				mountainHeight *= 60.0f;
				mountainHeight += myNoise.GetNoise((xx + x) * 1, (zz + z) * 1) * 5;
				mountainHeight = glm::pow(mountainHeight, 1.8f);
				if (std::isnan(mountainHeight)) {
					mountainHeight = 0;
				}

				y = myNoise.GetNoise((xx + x) * 1, (zz + z) * 1) * 15;
				y += mountainHeight;*/

				vertices->put(x)->put(y)->put(z);
				texCoords->put((1.0f / size) * i)->put((1.0f / size) * j);
			}
		}

		IntBuffer* indices = IntBuffer::allocate((((size - 1) * (size - 1)) * 3) * 2);
		for (int i = 0; i < size - 1; i++) {
			for (int j = 0; j < size - 1; j++) {
				int I0 = i + j * size;
				int I1 = (i + 1) + j * size;
				int J1 = i + (j + 1) * size;
				int IJ = (i + 1) + (j + 1) * size;
				indices->put(I0);
				indices->put(IJ);
				indices->put(J1);

				indices->put(I0);
				indices->put(I1);
				indices->put(IJ);
			}
		}

		FloatBuffer* normals = FloatBuffer::allocate(vertexCount);
		FloatBuffer* tangents = FloatBuffer::allocate(vertexCount);
		FloatBuffer* bitangents = FloatBuffer::allocate(vertexCount);

		for (int i = 0; i < indices->size() / 3; i++) {
			glm::vec3 p1 = glm::vec3(0, 0, 0);
			glm::vec3 p2 = glm::vec3(0, 0, 0);
			glm::vec3 p3 = glm::vec3(0, 0, 0);

			glm::vec2 uv0 = glm::vec2(0, 0);
			glm::vec2 uv1 = glm::vec2(0, 0);
			glm::vec2 uv2 = glm::vec2(0, 0);

			int n1 = indices->get(i * 3 + 0);
			int n2 = indices->get(i * 3 + 1);
			int n3 = indices->get(i * 3 + 2);

			populateFromBuffer(p1, vertices, n1);
			populateFromBuffer(p2, vertices, n2);
			populateFromBuffer(p3, vertices, n3);

			populateFromBuffer2(uv0, texCoords, n1);
			populateFromBuffer2(uv1, texCoords, n2);
			populateFromBuffer2(uv2, texCoords, n3);

			// NORMAL
			glm::vec3 U = p2 - p1;
			glm::vec3 V = p3 - p1;
			float nx = (U.y * V.z) - (U.z * V.y);
			float ny = (U.z * V.x) - (U.x * V.z);
			float nz = (U.x * V.y) - (U.y * V.x);
			glm::vec3 norm = glm::normalize(glm::vec3(nx, ny, nz));

			normals->put(n1 * 3 + 0, norm.x)->put(n1 * 3 + 1, norm.y)->put(n1 * 3 + 2, norm.z);
			normals->put(n2 * 3 + 0, norm.x)->put(n2 * 3 + 1, norm.y)->put(n2 * 3 + 2, norm.z);
			normals->put(n3 * 3 + 0, norm.x)->put(n3 * 3 + 1, norm.y)->put(n3 * 3 + 2, norm.z);

			// TANGENT SPACE
			glm::vec3 deltaPos1 = p2 - p1;
			glm::vec3 deltaPos2 = p3 - p1;

			glm::vec2 deltaUV1 = uv1 - uv0;
			glm::vec2 deltaUV2 = uv2 - uv0;

			float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
			glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y)*r;
			glm::vec3 bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x)*r;

			tangents->put(n1 * 3 + 0, tangent.x)->put(n1 * 3 + 1, tangent.y)->put(n1 * 3 + 2, tangent.z);
			tangents->put(n2 * 3 + 0, tangent.x)->put(n2 * 3 + 1, tangent.y)->put(n2 * 3 + 2, tangent.z);
			tangents->put(n3 * 3 + 0, tangent.x)->put(n3 * 3 + 1, tangent.y)->put(n3 * 3 + 2, tangent.z);

			bitangents->put(n1 * 3 + 0, bitangent.x)->put(n1 * 3 + 1, bitangent.y)->put(n1 * 3 + 2, bitangent.z);
			bitangents->put(n2 * 3 + 0, bitangent.x)->put(n2 * 3 + 1, bitangent.y)->put(n2 * 3 + 2, bitangent.z);
			bitangents->put(n3 * 3 + 0, bitangent.x)->put(n3 * 3 + 1, bitangent.y)->put(n3 * 3 + 2, bitangent.z);
		}

		Mesh* mesh = new Mesh();

		mesh->setData(MeshBuffer::Type::Position, 3, vertices);
		mesh->setData(MeshBuffer::Type::Normal, 3, normals);
		mesh->setData(MeshBuffer::Type::TexCoord, 2, texCoords);
		mesh->setData(MeshBuffer::Type::Index, 3, indices);
		mesh->setData(MeshBuffer::Type::Tangent, 3, tangents);
		mesh->setData(MeshBuffer::Type::BiTangent, 3, bitangents);
		mesh->updateBounds();
		mesh->createCollisionData();

		return mesh;
	}

	Mesh* generateMeshLevela(HeightStorage* heightStorage, int level, float cSize, glm::vec3 pos) {
		int triNum = 64;

		float dist = glm::abs(glm::distance(pos, glm::vec3(0, 0, 0)));

		/*if (dist > 1000) {
			triNum /= 8;
		} else if (dist > 800) {
			triNum /= 7;
		} else if (dist > 800) {
			triNum /= 6;
		} else if (dist > 600) {
			triNum /= 5;
		} else if (dist > 400) {
			triNum /= 4;
		} else if (dist > 300) {
			triNum /= 3;
		} else if (dist > 100) {
			triNum /= 2;
		}*/

		int triCount = triNum + 1;
		float patchSize = (float)cSize / (float)(triCount - 1);
		//std::cout << patchSize << std::endl;
		int size = triCount;
		return generateTerrainQuada(heightStorage, pos.x, pos.z, size, patchSize);
	}

	TerrainQuad::TerrainQuad(HeightStorage* heightStorage, Material* mat, Spatial* scene, float x, float z) : GameObject() {
		_heightStorage = heightStorage;
		//mat->getRenderState().wireframe = true;
		setMaterial(mat);
		setVisible(true);
		setLocation(glm::vec3(x, 0, z));
		setMesh(generateMeshLevela(heightStorage, 0, 128, getLocation()));
		setTemp(true);
	}

	void TerrainQuad::updateData() {
		delete getMesh();
		setMesh(generateMeshLevela(_heightStorage, 0, 128, getLocation()));
	}

}