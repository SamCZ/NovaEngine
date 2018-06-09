#ifndef TANGENT_GENERATOR_H
#define TANGENT_GENERATOR_H

#include "Engine/Render/Mesh/Mesh.h"
#include <vector>
#include "glm/glm.hpp"

namespace NovaEngine {

	const float ZERO_TOLERANCE = 0.0000001f;
	const float TOLERANCE_DOT = 0.99990604984064145942224512932761f;

	typedef struct VERTEX_INFO {
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 texCoord;
		std::vector<int> indices;
	} VertexInfo;

	typedef struct TRIANGLE_DATA {
		glm::vec3 tangent;
		glm::vec3 binormal;
		glm::vec3 normal;
		int index[3];
		int triangleOffset;

		void setIndex(int* i) {
			for (int a = 0; a < 3; a++) {
				index[a] = i[a];
			}
		}
	} TriangleData;

	typedef struct VERTEX_DATA {
		std::vector<TriangleData> triangles;
	} VertexData;

	class TangentGenerator {
	public:
		static void generateTangentAndBitangents(Mesh* mesh);

		static void generate(Mesh* mesh, bool approxTangents, bool splitMirrored);
		static std::vector<VertexData> processTriangles(Mesh* mesh, int* i, glm::vec3* v, glm::vec2* t, bool mir);
		static std::vector<VertexData> initVertexData(int size);
		static TriangleData processTriangle(int* index, glm::vec3* v, glm::vec2* t);
		static void processTriangleData(Mesh* mesh, std::vector<VertexData>& vertices, bool approxTangents, bool splitMirrored);
		static std::vector<VertexInfo> linkVertices(Mesh* mesh, bool splitMirrored);
	};
}

#endif // !TANGENT_GENERATOR_H