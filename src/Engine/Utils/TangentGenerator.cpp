#include "Engine/Utils/TangentGenerator.h"
#include "Engine/Utils/Buffers.h"
#include "glm/glm.hpp"

namespace NovaEngine {
	void TangentGenerator::generateTangentAndBitangents(Mesh* mesh) {
		if(mesh == nullptr) return;

		if(mesh->getMode() != Mode::Triangles) return;

		VertexBuffer* vib = mesh->getVertexBuffer(MeshBuffer::Type::Index);
		VertexBuffer* vpb = mesh->getVertexBuffer(MeshBuffer::Type::Position);
		VertexBuffer* ntb = mesh->getVertexBuffer(MeshBuffer::Type::Normal);
		VertexBuffer* vtb = mesh->getVertexBuffer(MeshBuffer::Type::TexCoord);

		if(vpb == nullptr || ntb == nullptr || vtb == nullptr) return;

		IntBuffer* indices = nullptr;
		FloatBuffer* vertices = static_cast<FloatBuffer*>(vpb->getBuffer());
		FloatBuffer* normals = static_cast<FloatBuffer*>(ntb->getBuffer());
		FloatBuffer* texCoords = static_cast<FloatBuffer*>(vtb->getBuffer());

		bool indexExists = false;

		int size;
		if (vib == nullptr) {
			size = vertices->size() / 3;
		} else {
			indexExists = true;
			indices = static_cast<IntBuffer*>(vib->getBuffer());
			size = indices->size();
		}

		glm::vec3 p1 = glm::vec3(0, 0, 0);
		glm::vec3 p2 = glm::vec3(0, 0, 0);
		glm::vec3 p3 = glm::vec3(0, 0, 0);

		glm::vec3 n1 = glm::vec3(0, 0, 0);
		glm::vec3 n2 = glm::vec3(0, 0, 0);
		glm::vec3 n3 = glm::vec3(0, 0, 0);

		glm::vec2 uv1 = glm::vec2(0, 0);
		glm::vec2 uv2 = glm::vec2(0, 0);
		glm::vec2 uv3 = glm::vec2(0, 0);

		FloatBuffer* tangents = FloatBuffer::allocate(indexExists ? indices->size() * 3 : vertices->size());
		FloatBuffer* bitangents = FloatBuffer::allocate(indexExists ? indices->size() * 3 : vertices->size());

		for (int i = 0; i < size / 3; i++) {
			int ii1 = i * 3 + 0;
			int ii2 = i * 3 + 1;
			int ii3 = i * 3 + 2;

			int i1 = indexExists ? indices->get(ii1) : ii1;
			int i2 = indexExists ? indices->get(ii2) : ii2;
			int i3 = indexExists ? indices->get(ii3) : ii3;

			populateFromBuffer(p1, vertices, i1);
			populateFromBuffer(p2, vertices, i2);
			populateFromBuffer(p3, vertices, i3);

			populateFromBuffer(n1, normals, i1);
			populateFromBuffer(n2, normals, i2);
			populateFromBuffer(n3, normals, i3);

			populateFromBuffer2(uv1, texCoords, i1);
			populateFromBuffer2(uv2, texCoords, i2);
			populateFromBuffer2(uv3, texCoords, i3);

			// TANGENT SPACE
			glm::vec3 deltaPos1 = p2 - p1;
			glm::vec3 deltaPos2 = p3 - p1;

			glm::vec2 deltaUV1 = uv2 - uv1;
			glm::vec2 deltaUV2 = uv3 - uv1;

			float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
			glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y)*r;
			glm::vec3 bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x)*r;

			tangents->put(i1 * 3 + 0, tangent.x)->put(i1 * 3 + 1, tangent.y)->put(i1 * 3 + 2, tangent.z);
			tangents->put(i2 * 3 + 0, tangent.x)->put(i2 * 3 + 1, tangent.y)->put(i2 * 3 + 2, tangent.z);
			tangents->put(i3 * 3 + 0, tangent.x)->put(i3 * 3 + 1, tangent.y)->put(i3 * 3 + 2, tangent.z);

			bitangents->put(i1 * 3 + 0, bitangent.x)->put(i1 * 3 + 1, bitangent.y)->put(i1 * 3 + 2, bitangent.z);
			bitangents->put(i2 * 3 + 0, bitangent.x)->put(i2 * 3 + 1, bitangent.y)->put(i2 * 3 + 2, bitangent.z);
			bitangents->put(i3 * 3 + 0, bitangent.x)->put(i3 * 3 + 1, bitangent.y)->put(i3 * 3 + 2, bitangent.z);
		}

		mesh->setData(MeshBuffer::Type::Tangent, 3, tangents);
		mesh->setData(MeshBuffer::Type::BiTangent, 3, bitangents);
	}
	void TangentGenerator::generate(Mesh* mesh, bool approxTangents, bool splitMirrored) {
		if (mesh->getMode() != Mode::Triangles) return;
		int index[3];
		glm::vec3 v[3];
		glm::vec2 t[3];
		
		std::vector<VertexData> vertices;

		vertices = processTriangles(mesh, index, v, t, splitMirrored);
		if(vertices.size() < 3) return;
		if (splitMirrored) {
			//splitVertices(mesh, vertices, splitMirrored);
		}
		processTriangleData(mesh, vertices, approxTangents, splitMirrored);
		//if the mesh has a bind pose, we need to generate the bind pose for the tangent buffer
		//TangentUtils.generateBindPoseTangentsIfNecessary(mesh);
	}
	std::vector<VertexData> TangentGenerator::processTriangles(Mesh * mesh, int* index, glm::vec3* v, glm::vec2* t, bool splitMirrored) {
		VertexBuffer* vib = mesh->getVertexBuffer(MeshBuffer::Type::Index);
		VertexBuffer* vpb = mesh->getVertexBuffer(MeshBuffer::Type::Position);
		VertexBuffer* ntb = mesh->getVertexBuffer(MeshBuffer::Type::Normal);
		VertexBuffer* vtb = mesh->getVertexBuffer(MeshBuffer::Type::TexCoord);

		if (vpb == nullptr || ntb == nullptr || vtb == nullptr) return std::vector<VertexData>();

		IntBuffer* indices = nullptr;
		FloatBuffer* vertices = static_cast<FloatBuffer*>(vpb->getBuffer());
		FloatBuffer* normals = static_cast<FloatBuffer*>(ntb->getBuffer());
		FloatBuffer* texCoords = static_cast<FloatBuffer*>(vtb->getBuffer());

		indices = static_cast<IntBuffer*>(vib->getBuffer());
		int size = indices->size();

		std::vector<VertexData> verts = initVertexData(vertices->size() / 3);
		for (int i = 0; i < size / 3; i++) {
			for (int j = 0; j < 3; j++) {
				index[j] = indices->get(i * 3 + j);
				populateFromBuffer(v[j], vertices, index[j]);
				populateFromBuffer2(t[j], texCoords, index[j]);
			}
			TriangleData triData = processTriangle(index, v, t);
			if (splitMirrored) {
				triData.setIndex(index);
				triData.triangleOffset = i * 3;
			}
			verts[index[0]].triangles.push_back(triData);
			verts[index[1]].triangles.push_back(triData);
			verts[index[2]].triangles.push_back(triData);
		}


		return verts;
	}
	std::vector<VertexData> TangentGenerator::initVertexData(int size) {
		std::vector<VertexData> verts;
		for (int i = 0; i < size; i++) {
			verts.push_back(VertexData());
		}
		return verts;
	}
	TriangleData TangentGenerator::processTriangle(int* index, glm::vec3* v, glm::vec2* t) {
		glm::vec3 edge1;
		glm::vec3 edge2;
		glm::vec2 edge1uv;
		glm::vec2 edge2uv;

		glm::vec3 tangent;
		glm::vec3 binormal;
		glm::vec3 normal;

		edge1uv = t[1] - t[0];
		edge2uv = t[2] - t[0];

		float det = edge1uv.x * edge2uv.y - edge1uv.y * edge2uv.x;

		bool normalize = false;
		if (glm::abs(det) < ZERO_TOLERANCE) {
			det = 1;
			normalize = true;
		}

		edge1 = v[1] - v[0];
		edge2 = v[2] - v[0];

		tangent = glm::normalize(edge1);
		binormal = glm::normalize(edge2);

		if (glm::abs(glm::abs(glm::dot(tangent, binormal)) - 1)
			< ZERO_TOLERANCE) {
	
		}

		float factor = 1 / det;
		tangent.x = (edge2uv.y * edge1.x - edge1uv.y * edge2.x) * factor;
		tangent.y = (edge2uv.y * edge1.y - edge1uv.y * edge2.y) * factor;
		tangent.z = (edge2uv.y * edge1.z - edge1uv.y * edge2.z) * factor;
		if (normalize) {
			tangent = glm::normalize(tangent);
		}

		binormal.x = (edge1uv.x * edge2.x - edge2uv.x * edge1.x) * factor;
		binormal.y = (edge1uv.x * edge2.y - edge2uv.x * edge1.y) * factor;
		binormal.z = (edge1uv.x * edge2.z - edge2uv.x * edge1.z) * factor;
		if (normalize) {
			binormal = glm::normalize(binormal);
		}

		normal = glm::cross(tangent, binormal);
		normal = glm::normalize(normal);

		TriangleData data;
		data.tangent = tangent;
		data.binormal = binormal;
		data.normal = normal;
		return data;
	}

	bool approxEqual(glm::vec3 u, glm::vec3 v) {
		float tolerance = 1E-4f;
		return (glm::abs(u.x - v.x) < tolerance) && (glm::abs(u.y - v.y) < tolerance) && (glm::abs(u.z - v.z) < tolerance);
	}

	bool approxEqual(glm::vec2 u, glm::vec2 v) {
		float tolerance = 1E-4f;
		return (glm::abs(u.x - v.x) < tolerance) && (glm::abs(u.y - v.y) < tolerance);
	}

	std::vector<VertexInfo> TangentGenerator::linkVertices(Mesh* mesh, bool splitMirrored) {
		VertexBuffer* vib = mesh->getVertexBuffer(MeshBuffer::Type::Index);
		VertexBuffer* vpb = mesh->getVertexBuffer(MeshBuffer::Type::Position);
		VertexBuffer* ntb = mesh->getVertexBuffer(MeshBuffer::Type::Normal);
		VertexBuffer* vtb = mesh->getVertexBuffer(MeshBuffer::Type::TexCoord);

		IntBuffer* indices = static_cast<IntBuffer*>(vib->getBuffer());
		FloatBuffer* vertices = static_cast<FloatBuffer*>(vpb->getBuffer());
		FloatBuffer* normals = static_cast<FloatBuffer*>(ntb->getBuffer());
		FloatBuffer* texCoords = static_cast<FloatBuffer*>(vtb->getBuffer());

		int size = vertices->size() / 3;

		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 texCoord;

		std::vector<VertexInfo> vertexMap;
		for (int i = 0; i < size; i++) {
			populateFromBuffer(position, vertices, i);
			populateFromBuffer(normal, normals, i);
			populateFromBuffer2(texCoord, texCoords, i);

			bool found = false;

			if (!splitMirrored) {
				for (int j = 0; j < vertexMap.size(); j++) {
					VertexInfo& vertexInfo = vertexMap[j];
					if (approxEqual(vertexInfo.position, position) && approxEqual(vertexInfo.normal, normal) && approxEqual(vertexInfo.texCoord, texCoord)) {
						vertexInfo.indices.push_back(i);
						found = true;
						break;
					}
				}
			}
			if (!found) {
				VertexInfo vertexInfo;
				vertexInfo.position = position;
				vertexInfo.normal = normal;
				vertexInfo.texCoord = texCoord;

				vertexInfo.indices.push_back(i);
				vertexMap.push_back(vertexInfo);
			}
		}

		return vertexMap;
	}

	void TangentGenerator::processTriangleData(Mesh* mesh, std::vector<VertexData>& vertices, bool approxTangent, bool splitMirrored) {
		std::vector<VertexInfo> vertexMap = linkVertices(mesh, splitMirrored);
		if(vertexMap.size() == 0) return;
		
		FloatBuffer* tangents = FloatBuffer::allocate(vertices.size() * 4);
		//FloatBuffer* bitangents = FloatBuffer::allocate(indexExists ? indices->size() * 3 : vertices->size());

		glm::vec3 tangent;
		glm::vec3 binormal;
		glm::vec3 givenNormal;

		glm::vec3 tangentUnit;
		glm::vec3 binormalUnit;
		for (int k = 0; k < vertexMap.size(); k++) {
			float wCoord = -1;

			VertexInfo vertexInfo = vertexMap[k];
			givenNormal = glm::normalize(vertexInfo.normal);

			TriangleData firstTriangle = vertices[vertexInfo.indices[0]].triangles[0];
			// check tangent and binormal consistency
			tangent = glm::normalize(firstTriangle.tangent);
			binormal = glm::normalize(firstTriangle.binormal);

			for (int i : vertexInfo.indices) {
				std::vector<TriangleData>& triangles = vertices[i].triangles;

				for (int j = 0; j < triangles.size(); j++) {
					TriangleData triangleData = triangles[j];

					tangentUnit = glm::normalize(triangleData.tangent);

					if (glm::dot(tangent, tangentUnit) < TOLERANCE_DOT) {
						break;
					}

					if (!approxTangent) {
						binormalUnit = glm::normalize(triangleData.binormal);
						if (glm::dot(binormal, binormalUnit) < TOLERANCE_DOT) {
							break;
						}
					}
				}
			}

			// find average tangent
			tangent = glm::vec3(0.0f);
			binormal = glm::vec3(0.0f);

			int triangleCount = 0;

			for (int i : vertexInfo.indices) {
				std::vector<TriangleData>& triangles = vertices[i].triangles;
				triangleCount += triangles.size();

				for (int j = 0; j < triangles.size(); j++) {
					TriangleData triangleData = triangles[j];
					tangent += triangleData.tangent;
					binormal += triangleData.binormal;

				}
			}

			int blameVertex = vertexInfo.indices[0];

			if (tangent.length() < ZERO_TOLERANCE) {
				// attempt to fix from binormal
				if (binormal.length() >= ZERO_TOLERANCE) {
					tangent = glm::normalize(glm::cross(binormal, givenNormal));
				} // if all fails use the tangent from the first triangle
				else {
					tangent = firstTriangle.tangent;
				}
			} else {
				tangent /= triangleCount;
			}

			tangentUnit = glm::normalize(tangent);

			/*if (Math.abs(Math.abs(tangentUnit.dot(givenNormal)) - 1) < ZERO_TOLERANCE) {
			}*/
			
			if (!approxTangent) {
				if (binormal.length() < ZERO_TOLERANCE) {
					// attempt to fix from tangent
					if (tangent.length() >= ZERO_TOLERANCE) {
						binormal = glm::normalize(glm::cross(givenNormal, tangent));
					} // if all fails use the binormal from the first triangle
					else {
						binormal = firstTriangle.binormal;
					}
				} else {
					binormal /= triangleCount;
				}

				binormalUnit = glm::normalize(binormal);
				/*if (Math.abs(Math.abs(binormalUnit.dot(givenNormal)) - 1)
					< ZERO_TOLERANCE) {
				}

				if (Math.abs(Math.abs(binormalUnit.dot(tangentUnit)) - 1)
					< ZERO_TOLERANCE) {
				}*/
			}

			glm::vec3 finalTangent;
			glm::vec3 tmp;
			for (int i : vertexInfo.indices) {
				if (approxTangent) {
					// Gram-Schmidt orthogonalize
					tmp = givenNormal * glm::dot(givenNormal, tangent);
					finalTangent = glm::normalize(tangent - tmp);

					wCoord = glm::dot(glm::cross(givenNormal, tangent), binormal) < 0.0f ? -1.0f : 1.0f;

					tangents->put((i * 4), finalTangent.x);
					tangents->put((i * 4) + 1, finalTangent.y);
					tangents->put((i * 4) + 2, finalTangent.z);
					tangents->put((i * 4) + 3, wCoord);
				} else {
					tangents->put((i * 4), tangent.x);
					tangents->put((i * 4) + 1, tangent.y);
					tangents->put((i * 4) + 2, tangent.z);
					tangents->put((i * 4) + 3, wCoord);

					//setInBuffer(binormal, binormals, i);
				}
			}
		}
		mesh->setData(MeshBuffer::Type::Tangent, 4, tangents);
	}
}