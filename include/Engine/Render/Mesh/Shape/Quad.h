#ifndef QUAD_H
#define QUAD_H

#include "Engine/Render/Mesh/Mesh.h"
#include "Engine/Utils/Buffers.h"

namespace NovaEngine {
	class Quad : public Mesh {
	public:
		inline ~Quad() {}
		inline Quad(float width, float height, bool flipCoords, bool plane = false) {
			if (plane) {
				float hw = width * 0.5f;
				float hh = height * 0.5f;
				setData(MeshBuffer::Type::Position, 3, createFloatBuffer(new float[12]{
					-hw, 0, -hh,
					-hw, 0, hh,
					hw, 0, hh,
					hw, 0, -hh
				}, 12));
				setData(MeshBuffer::Type::TexCoord, 2, createFloatBuffer(new float[8]{ 0, 0, 1, 0, 1, 1, 0, 1 }, 8));
				setData(MeshBuffer::Type::Normal, 3, createFloatBuffer(new float[12]{
					0, 1, 0,
					0, 1, 0,
					0, 1, 0,
					0, 1, 0,
				}, 12));

				if (height < 0) {
					setData(MeshBuffer::Type::Index, 1, createIntBuffer(new int[6]{ 0, 2, 1, 0, 3, 2 }, 6));
				} else {
					setData(MeshBuffer::Type::Index, 1, createIntBuffer(new int[6]{ 0, 1, 2, 0, 2, 3 }, 6));
				}
			} else {
				setData(MeshBuffer::Type::Position, 3, createFloatBuffer(new float[12]{ 0, 0, 0, width, 0, 0, width, height, 0, 0, height, 0 }, 12));

				if (flipCoords) {
					setData(MeshBuffer::Type::TexCoord, 2, createFloatBuffer(new float[8]{ 0, 1, 1, 1, 1, 0, 0, 0 }, 8));
				} else {
					setData(MeshBuffer::Type::TexCoord, 2, createFloatBuffer(new float[8]{ 0, 0, 1, 0, 1, 1, 0, 1 }, 8));
				}

				setData(MeshBuffer::Type::Normal, 3, createFloatBuffer(new float[12]{ 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1 }, 12));

				if (height < 0) {
					setData(MeshBuffer::Type::Index, 1, createIntBuffer(new int[6]{ 0, 2, 1, 0, 3, 2 }, 6));
				} else {
					setData(MeshBuffer::Type::Index, 1, createIntBuffer(new int[6]{ 0, 1, 2, 0, 2, 3 }, 6));
				}
			}
			updateBounds();
		}
	};
}

#endif // !QUAD_H