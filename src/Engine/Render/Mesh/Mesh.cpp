#include "Engine/Render/Mesh/Mesh.h"
#include "Engine/Collisions/BoundingBox.h"
#include "Engine/Collisions/BIHTree.h"

namespace NovaEngine {
	Mesh::Mesh() : _vertexBuffersArr(new VertexBuffer*[8]) {
		_mode = Mode::Triangles;
		_patchVertexCount = 0;
		_enabledVertexAttribArrays = 0;
		for (int i = 0; i < 8; i++) {
			_vertexBuffersArr[i] = nullptr;
		}
	}

	Mesh::~Mesh() {
		delete _boundingBox;
		delete _collisionTree;

		for (int i = 0; i < 8; i++) {
			if (_vertexBuffersArr[i]) {
				delete _vertexBuffersArr[i];
			}
		}
		delete[] _vertexBuffersArr;
	}

	void Mesh::setData(MeshBuffer::Type type, int components, Buffer* buffer) {
		if (_vertexBuffersArr[(int)type] != nullptr) {
			_vertexBuffersArr[(int)type]->updateData(buffer);
		} else {
			VertexBuffer* vb = new VertexBuffer(type);
			MeshBuffer::Format format;
			switch (buffer->getType()) {
				case BufferType::Float:
				format = MeshBuffer::Format::Float;
				break;
				case BufferType::Int:
				format = MeshBuffer::Format::Int;
				break;
			}
			vb->setupData(MeshBuffer::Usage::Dynamic, components, format, buffer);
			_vertexBuffersArr[(int)type] = vb;
		}
	}

	void Mesh::setVertexBuffer(MeshBuffer::Type type, VertexBuffer* buffer) {
		_vertexBuffersArr[(int)type] = buffer;
	}

	VertexBuffer* Mesh::getVertexBuffer(MeshBuffer::Type type) {
		return _vertexBuffersArr[(int)type];
	}

	VertexBuffer** Mesh::getVertexBuffers() {
		return _vertexBuffersArr;
	}

	Mode Mesh::getMode() const {
		return _mode;
	}
	void Mesh::setMode(Mode mode) {
		_mode = mode;
	}

	void Mesh::createCollisionData() {
		BIHTree* ct = new BIHTree(this);
		ct->construct();
		_collisionTree = ct;
	}

	BIHTree* Mesh::getCollider() {
		return _collisionTree;
	}

	BoundingBox* Mesh::getBounds() {
		return _boundingBox;
	}

	void Mesh::setBounds(BoundingBox * bound) {
		_boundingBox = bound;
	}

	void Mesh::updateBounds() {
		if (_boundingBox == nullptr) {
			_boundingBox = new BoundingBox();
		}
		_boundingBox->computeFromMesh(this);
	}

	int Mesh::getPatchVertexCount() const {
		return _patchVertexCount;
	}

	void Mesh::setPatchVertexCount(int patchCount) {
		_patchVertexCount = patchCount;
	}
	int Mesh::getInstanceCount() const {
		return _instanceCount;
	}
	void Mesh::setInstanceCount(int count) {
		_instanceCount = count;
	}
}