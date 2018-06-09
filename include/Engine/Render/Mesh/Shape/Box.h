#ifndef SHAPE_BOX_H
#define SHAPE_BOX_H

#include "Engine/Render/Mesh/Mesh.h"
#include "Engine/Utils/Buffers.h"

namespace NovaEngine {
	class Box : public Mesh {
	public:
		inline ~Box() {}
		inline Box(glm::vec3 center, float xSize, float ySize, float zSize) : Mesh() {
			int indices[12*3] = {
				0,1,2,
				0,2,3,
				4,5,6,
				4,6,7,
				8,9,10,
				8,10,11,
				12,13,14,
				12,14,15,
				16,17,18,
				16,18,19,
				20,21,22,
				20,22,23
			};

			float vertexBuffer[24 * 3] = {
				xSize,-ySize,-zSize,
				xSize,-ySize,zSize,
			   -xSize,-ySize,zSize,
				-xSize,-ySize,-zSize,
				xSize,ySize,-zSize,
				-xSize,ySize,-zSize,
				-xSize,ySize,zSize,
				xSize,ySize,zSize,
				xSize,-ySize,-zSize,
				xSize,ySize,-zSize,
				xSize,ySize,zSize,
				xSize,-ySize,zSize,
				xSize,-ySize,zSize,
				xSize,ySize,zSize,
				-xSize,ySize,zSize,
				-xSize,-ySize,zSize,
				-xSize,-ySize,zSize,
				-xSize,ySize,zSize,
				-xSize,ySize,-zSize,
				-xSize,-ySize,-zSize,
				xSize,ySize,-zSize,
				xSize,-ySize,-zSize,
				-xSize,-ySize,-zSize,
				-xSize,ySize,-zSize
			};
			/*float vertexBuffer[24 * 3] = {
				1,-1,-1,
				1,-1,1,
				-1,-1,1,
				-1,-1,-1,
				1,1,-0.999999,
				-1,1,-1,
				-1,1,1,
				0.999999,1,1,
				1,-1,-1,
				1,1,-0.999999,
				0.999999,1,1,
				1,-1,1,
				1,-1,1,
				0.999999,1,1,
				-1,1,1,
				-1,-1,1,
				-1,-1,1,
				-1,1,1,
				-1,1,-1,
				-1,-1,-1,
				1,1,-0.999999,
				1,-1,-1,
				-1,-1,-1,
				-1,1,-1
			};*/

			float textureBuffer[24 * 2] = {
				0,1,
				1,1,
				1,0,
				0,0,
				0,1,
				1,1,
				1,0,
				0,0,
				0,1,
				1,1,
				1,0,
				0,0,
				0,1,
				1,1,
				1,0,
				0,0,
				0,1,
				1,1,
				1,0,
				0,0,
				0,1,
				1,1,
				1,0,
				0,0
			};

			float normalBuffer[24 * 3] = {
				0,-1,0,
				0,-1,0,
				0,-1,0,
				0,-1,0,
				0,1,0,
				0,1,0,
				0,1,0,
				0,1,0,
				1,0,0,
				1,0,0,
				1,0,0,
				1,0,0,
				-0,-0,1,
				-0,-0,1,
				-0,-0,1,
				-0,-0,1,
				-1,-0,-0,
				-1,-0,-0,
				-1,-0,-0,
				-1,-0,-0,
				0,0,-1,
				0,0,-1,
				0,0,-1,
				0,0,-1
			};

			IntBuffer* ib = IntBuffer::allocate(12*3);
			ib->put(indices, 12*3);
			setData(MeshBuffer::Type::Index, 3, ib);

			FloatBuffer* fb = FloatBuffer::allocate(24*3);
			fb->put(vertexBuffer, 24*3);
			setData(MeshBuffer::Type::Position, 3, fb);

			FloatBuffer* nb = FloatBuffer::allocate(24*3);
			nb->put(normalBuffer, 24*3);
			setData(MeshBuffer::Type::Normal, 3, nb);

			FloatBuffer* tb = FloatBuffer::allocate(24*2);
			tb->put(textureBuffer, 24*2);
			setData(MeshBuffer::Type::TexCoord, 2, tb);

			updateBounds();
		}

		inline Box(float xSize, float ySize, float zSize) : Box(glm::vec3(), xSize, ySize, zSize) {}
	};
}

#endif // !SHAPE_BOX_H