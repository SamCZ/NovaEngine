#include "Engine/Utils/Buffers/FloatBuffer.h"

#include <iostream>

namespace NovaEngine {
	FloatBuffer::FloatBuffer(int size) {
		_data = new float[size];
		_size = size;
		_nextIndex = 0;
	}

	FloatBuffer::~FloatBuffer() {
		delete[] _data;
	}

	FloatBuffer* FloatBuffer::allocate(int size) {
		return new FloatBuffer(size);
	}

	FloatBuffer* FloatBuffer::allocate3(std::vector<glm::vec3> vec) {
		FloatBuffer* buff = FloatBuffer::allocate(vec.size() * 3);
		for(glm::vec3& v : vec) {
			buff->put(v.x)->put(v.y)->put(v.z);
		}
		return buff;
	}

	FloatBuffer* FloatBuffer::allocate2(std::vector<glm::vec2> vec) {
		FloatBuffer* buff = FloatBuffer::allocate(vec.size() * 2);
		for (glm::vec2& v : vec) {
			buff->put(v.x)->put(v.y);
		}
		return buff;
	}

	void FloatBuffer::clear() {
		memset(_data, 0, _size * sizeof(float));
		rewind();
	}

	FloatBuffer* FloatBuffer::put(float f) {
		int index = _nextIndex++;
		if (index > _size - 1) {
			std::cerr << "Buffer overflow exception ! (" << index << ")" << std::endl;
			return this;
		}
		_data[index] = f;
		return this;
	}

	FloatBuffer* FloatBuffer::put(int index, float f) {
		if (index > _size - 1) {
			std::cerr << "Buffer overflow exception ! (" << index << ")" << std::endl;
			return this;
		}
		_data[index] = f;
		return this;
	}

	FloatBuffer * FloatBuffer::put(float* data, int size) {
		for (int i = 0; i < size; i++) {
			put(data[i]);
		}
		return this;
	}

	float FloatBuffer::get() {
		return _data[_nextIndex];
	}

	float FloatBuffer::get(int index) {
		return _data[index];
	}

	float* FloatBuffer::getData() {
		return _data;
	}
	void FloatBuffer::print(int c) {
		for (int i = 0; i < size() / c; i++) {
			for (int a = 0; a < c; a++) {
				std::cout << get(i * c + a) << ",";
			}
			std::cout << std::endl;
		}
	}
}