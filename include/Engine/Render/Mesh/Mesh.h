#ifndef MESH_H
#define MESH_H

#include <map>
#include "Engine/Render/Mesh/VertexBuffer.h"

namespace NovaEngine {
	class BoundingBox;
	class BIHTree;

	enum class Mode : GLenum {
		Triangles = GL_TRIANGLES,
		Quads = GL_QUADS,
		TriangleTrip = GL_TRIANGLE_STRIP,
		TriangleFan = GL_TRIANGLE_FAN,
		Lines = GL_LINES,
		Points = GL_POINTS,
		Patch = GL_PATCHES
	};

	class Mesh {
	public:
		GLuint _vertexAttribArrays[16];
		int _enabledVertexAttribArrays = 0;
		GLuint _lastShader;

		Mesh();
		~Mesh();

		void setData(MeshBuffer::Type type, int components, Buffer* buffer);

		void setVertexBuffer(MeshBuffer::Type type, VertexBuffer* buffer);
		VertexBuffer* getVertexBuffer(MeshBuffer::Type type);

		VertexBuffer** getVertexBuffers();

		Mode getMode() const;
		void setMode(Mode mode);

		void createCollisionData();
		BIHTree* getCollider();

		BoundingBox* getBounds();
		void setBounds(BoundingBox* bound);
		void updateBounds();

		int getPatchVertexCount() const;
		void setPatchVertexCount(int patchCount);

		int getInstanceCount() const;
		void setInstanceCount(int count);
	private:
		Mode _mode;
		VertexBuffer** _vertexBuffersArr;
		BIHTree* _collisionTree = nullptr;
		BoundingBox* _boundingBox = nullptr;
		int _patchVertexCount;
		int _instanceCount;
	};
}

#endif // !MESH_H