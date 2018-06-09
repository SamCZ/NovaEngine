#ifndef CUBEMAP_BOX
#define CUBEMAP_BOX

#include "Engine/Render/Mesh/Mesh.h"
#include "Engine/Utils/Buffers.h"

namespace NovaEngine {
	class CubeMapBox : public Mesh {
	public:
		inline ~CubeMapBox() {

		}
		inline CubeMapBox() : Mesh() {
			float SIZE = 1000.0f;

			float* VERTICES = new float[108] {
				-SIZE,  SIZE, -SIZE,
				-SIZE, -SIZE, -SIZE,
				SIZE, -SIZE, -SIZE,
				SIZE, -SIZE, -SIZE,
				SIZE,  SIZE, -SIZE,
				-SIZE,  SIZE, -SIZE,

				-SIZE, -SIZE,  SIZE,
				-SIZE, -SIZE, -SIZE,
				-SIZE,  SIZE, -SIZE,
				-SIZE,  SIZE, -SIZE,
				-SIZE,  SIZE,  SIZE,
				-SIZE, -SIZE,  SIZE,

				SIZE, -SIZE, -SIZE,
				SIZE, -SIZE,  SIZE,
				SIZE,  SIZE,  SIZE,
				SIZE,  SIZE,  SIZE,
				SIZE,  SIZE, -SIZE,
				SIZE, -SIZE, -SIZE,

				-SIZE, -SIZE,  SIZE,
				-SIZE,  SIZE,  SIZE,
				SIZE,  SIZE,  SIZE,
				SIZE,  SIZE,  SIZE,
				SIZE, -SIZE,  SIZE,
				-SIZE, -SIZE,  SIZE,

				-SIZE,  SIZE, -SIZE,
				SIZE,  SIZE, -SIZE,
				SIZE,  SIZE,  SIZE,
				SIZE,  SIZE,  SIZE,
				-SIZE,  SIZE,  SIZE,
				-SIZE,  SIZE, -SIZE,

				-SIZE, -SIZE, -SIZE,
				-SIZE, -SIZE,  SIZE,
				SIZE, -SIZE, -SIZE,
				SIZE, -SIZE, -SIZE,
				-SIZE, -SIZE,  SIZE,
				SIZE, -SIZE,  SIZE
			};

			FloatBuffer* fb = FloatBuffer::allocate(108);
			fb->put(VERTICES, 108);
			delete[] VERTICES;

			this->setData(MeshBuffer::Type::Position, 3, fb);
		}
	};
}

#endif // !CUBEMAP_BOX