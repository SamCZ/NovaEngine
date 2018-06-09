#ifndef QUAD_BRDF_H
#define QUAD_BRDF_H

#include "Engine/Render/Mesh/Mesh.h"
#include "Engine/Utils/Buffers.h"

namespace NovaEngine {
	class QuadBRDF : public Mesh {
	public:
		inline ~QuadBRDF() {}
		inline QuadBRDF() {
			setData(MeshBuffer::Type::Position, 3, FloatBuffer::allocate3({ 
				glm::vec3(-1.0f,  1.0f, 0.0f),
				glm::vec3(-1.0f, -1.0f, 0.0f),
				glm::vec3(1.0f,  1.0f, 0.0f),
				glm::vec3(1.0f, -1.0f, 0.0f)
			}));
			setData(MeshBuffer::Type::TexCoord, 2, FloatBuffer::allocate2({
				glm::vec2(0.0f, 1.0f),
				glm::vec2(0.0f, 0.0f),
				glm::vec2(1.0f, 1.0f),
				glm::vec2(1.0f, 0.0f)
			}));
			setMode(Mode::TriangleTrip);
		}
	};
}

#endif // !QUAD_BRDF_H