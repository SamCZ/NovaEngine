#include "Engine/Render/Mesh/VertexBuffer.h"
#include <iostream>

namespace NovaEngine {
	VertexBuffer::VertexBuffer(MeshBuffer::Type type) {
		_type = type;
		_buffer = nullptr;
	}

	VertexBuffer::~VertexBuffer() {
		try {
			if (_buffer) {
				delete _buffer;
				_buffer = NULL;
			}
		} catch (int e) {

		}
		glDeleteBuffers(1, &_id);
	}

	void VertexBuffer::setupData(MeshBuffer::Usage usage, int components, MeshBuffer::Format format, Buffer* buffer) {
		if (_id != -1) {
			std::cerr << "Data has already been sent. Cannot setupData again." << std::endl;
			return;
		}
		_usage = usage;
		_components = components;
		_format = format;
		_buffer = buffer;
		markUpdate();
	}

	void VertexBuffer::updateData(Buffer* buffer) {
		_buffer = buffer;
		markUpdate();
	}

	void VertexBuffer::setUsage(MeshBuffer::Usage usage) {
		_usage = usage;
		markUpdate();
	}

	MeshBuffer::Type VertexBuffer::getType() const {
		return _type;
	}

	MeshBuffer::Usage VertexBuffer::getUsage() const {
		return _usage;
	}

	MeshBuffer::Format VertexBuffer::getFormat() const {
		return _format;
	}

	int VertexBuffer::getNumComponents() const {
		return _components;
	}

	Buffer* VertexBuffer::getBuffer() {
		return _buffer;
	}
}