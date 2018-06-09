#ifndef SPHERE_H
#define SPHERE_H

#include <vector>
#include "Engine/Render/Mesh/Mesh.h"
#include "glm/glm.hpp"
#include "Engine/Utils/Buffers.h"
#include "Engine/Utils/Math.h"

namespace NovaEngine {
	enum class TextureMode {
		Original,
		Projected,
		Polar
	};

	class Sphere : public Mesh {
	private:
		TextureMode _textureMode;
	public:
		inline Sphere(int zSamples, int radialSamples, float radius, bool useEvenSlices, bool interior) : Mesh() {
			_textureMode = TextureMode::Projected;
			int vertCount = (zSamples - 2) * (radialSamples + 1) + 2;

			std::vector<glm::vec3> vertices;
			std::vector<glm::vec3> normals;
			std::vector<glm::vec2> texCoords;

			float fInvRS = 1.0f / radialSamples;
			float fZFactor = 2.0f / (zSamples - 1);

			float* afSin = new float[(radialSamples + 1)];
			float* afCos = new float[(radialSamples + 1)];
			for (int iR = 0; iR < radialSamples; iR++) {
				float fAngle = M_PI_2 * fInvRS * iR;
				afCos[iR] = glm::cos(fAngle);
				afSin[iR] = glm::sin(fAngle);
			}
			afSin[radialSamples] = afSin[0];
			afCos[radialSamples] = afCos[0];

			glm::vec3 tempVa = glm::vec3(0, 0, 0);
			glm::vec3 tempVb = glm::vec3(0, 0, 0);
			glm::vec3 tempVc = glm::vec3(0, 0, 0);

			// generate the sphere itself
			int i = 0;
			for (int iZ = 1; iZ < (zSamples - 1); iZ++) {
				float fAFraction = HALF_PI * (-1.0f + fZFactor * iZ); // in (-pi/2, pi/2)
				float fZFraction;
				if (useEvenSlices) {
					fZFraction = -1.0f + fZFactor * iZ; // in (-1, 1)
				} else {
					fZFraction = glm::sin(fAFraction); // in (-1,1)
				}
				float fZ = radius * fZFraction;

				// compute center of slice
				glm::vec3 kSliceCenter = (tempVb = glm::vec3(0, 0, 0));
				kSliceCenter.z += fZ;

				// compute radius of slice
				float fSliceRadius = glm::sqrt(glm::abs(radius * radius
											   - fZ * fZ));

				// compute slice vertices with duplication at end point
				glm::vec3 kNormal;
				int iSave = i;
				for (int iR = 0; iR < radialSamples; iR++) {
					float fRadialFraction = iR * fInvRS; // in [0,1)
					glm::vec3 kRadial = (tempVc = glm::vec3(afCos[iR], afSin[iR], 0));
					tempVa = kRadial * fSliceRadius;
					tempVa = glm::vec3(kSliceCenter.x + tempVa.x, kSliceCenter.y + tempVa.y, kSliceCenter.z + tempVa.z);
					vertices.push_back(tempVa);

					kNormal = tempVa;
					kNormal = glm::normalize(kNormal);
					if (!interior) { // allow interior texture vs. exterior
						normals.push_back(glm::vec3(kNormal.x, kNormal.y, kNormal.z));
					} else {
						normals.push_back(glm::vec3(-kNormal.x, -kNormal.y, -kNormal.z));
					}
					
					if (_textureMode == TextureMode::Original) {
						texCoords.push_back(glm::vec2(fRadialFraction, 0.5f * (fZFraction + 1.0f)));
					} else if (_textureMode == TextureMode::Projected) {
						texCoords.push_back(glm::vec2(fRadialFraction, INV_PI * (HALF_PI + glm::asin(fZFraction))));
					} else if (_textureMode == TextureMode::Polar) {
						float r = (HALF_PI - glm::abs(fAFraction)) / M_PI;
						float u = r * afCos[iR] + 0.5f;
						float v = r * afSin[iR] + 0.5f;
						texCoords.push_back(glm::vec2(u, v));
					}

					i++;
				}

				vertices.insert(vertices.begin() + i, vertices[iSave]);
				normals.insert(normals.begin() + i, normals[iSave]);

				if (_textureMode == TextureMode::Original) {
					texCoords.push_back(glm::vec2(1.0f, 0.5f * (fZFraction + 1.0f)));
				} else if (_textureMode == TextureMode::Projected) {
					texCoords.push_back(glm::vec2(1.0f, INV_PI * (HALF_PI + glm::asin(fZFraction))));
				} else if (_textureMode == TextureMode::Polar) {
					float r = (HALF_PI - glm::abs(fAFraction)) / M_PI;
					texCoords.push_back(glm::vec2(r + 0.5f, 0.5f));
				}

				i++;
			}

			// south pole
			vertices.insert(vertices.begin() + i, glm::vec3(0, 0, -radius));
			if (!interior) {
				normals.insert(normals.begin() + i, glm::vec3(0, 0, -1));
			} else {
				normals.insert(normals.begin() + i, glm::vec3(0, 0, 1));
			}

			if (_textureMode == TextureMode::Polar) {
				texCoords.insert(texCoords.begin() + i, glm::vec2(0.5f, 0.5f));
			} else {
				texCoords.insert(texCoords.begin() + i, glm::vec2(0.5f, 0));
			}
			i++;

			// north pole
			vertices.insert(vertices.begin() + i, glm::vec3(0, 0, radius));
			if (!interior) {
				normals.insert(normals.begin() + i, glm::vec3(0, 0, 1));
			} else {
				normals.insert(normals.begin() + i, glm::vec3(0, 0, -1));
			}

			if (_textureMode == TextureMode::Polar) {
				texCoords.insert(texCoords.begin() + i, glm::vec2(0.5f, 0.5f));
			} else {
				texCoords.insert(texCoords.begin() + i, glm::vec2(0.5f, 1.0f));
			}

			FloatBuffer* b_vertices = FloatBuffer::allocate3(vertices);
			FloatBuffer* b_normals = FloatBuffer::allocate3(normals);
			FloatBuffer* b_texCoords = FloatBuffer::allocate2(texCoords);

			setData(MeshBuffer::Type::Position, 3, b_vertices);
			setData(MeshBuffer::Type::Normal, 3, b_normals);
			setData(MeshBuffer::Type::TexCoord, 2, b_texCoords);

			int triCount = 2 * (zSamples - 2) * radialSamples;
			std::vector<unsigned int> idxBuf;

			// generate connectivity
			int index = 0;
			for (int iZ = 0, iZStart = 0; iZ < (zSamples - 3); iZ++) {
				int i0 = iZStart;
				int i1 = i0 + 1;
				iZStart += (radialSamples + 1);
				int i2 = iZStart;
				int i3 = i2 + 1;
				for (int i = 0; i < radialSamples; i++, index += 6) {
					if (!interior) {
						idxBuf.push_back((short)i0++);
						idxBuf.push_back((short)i1);
						idxBuf.push_back((short)i2);
						idxBuf.push_back((short)i1++);
						idxBuf.push_back((short)i3++);
						idxBuf.push_back((short)i2++);
					} else { // inside view
						idxBuf.push_back((short)i0++);
						idxBuf.push_back((short)i2);
						idxBuf.push_back((short)i1);
						idxBuf.push_back((short)i1++);
						idxBuf.push_back((short)i2++);
						idxBuf.push_back((short)i3++);
					}
				}
			}

			// south pole triangles
			for (int i = 0; i < radialSamples; i++, index += 3) {
				if (!interior) {
					idxBuf.push_back((short)i);
					idxBuf.push_back((short)(vertCount - 2));
					idxBuf.push_back((short)(i + 1));
				} else { // inside view
					idxBuf.push_back((short)i);
					idxBuf.push_back((short)(i + 1));
					idxBuf.push_back((short)(vertCount - 2));
				}
			}

			// north pole triangles
			int iOffset = (zSamples - 3) * (radialSamples + 1);
			for (int i = 0; i < radialSamples; i++, index += 3) {
				if (!interior) {
					idxBuf.push_back((short)(i + iOffset));
					idxBuf.push_back((short)(i + 1 + iOffset));
					idxBuf.push_back((short)(vertCount - 1));
				} else { // inside view
					idxBuf.push_back((short)(i + iOffset));
					idxBuf.push_back((short)(vertCount - 1));
					idxBuf.push_back((short)(i + 1 + iOffset));
				}
			}
			setData(MeshBuffer::Type::Index, 3, IntBuffer::allocate(idxBuf));
			updateBounds();
		}
	};
}

#endif // !SPHERE_H