#include "Engine/Utils/Buffers/IntBuffer.h"

#include <iostream>

namespace NovaEngine {
	IntBuffer::IntBuffer(int size) {
		_data = new int[size];
		_size = size;
	}

	IntBuffer::~IntBuffer() {
		delete[] _data;
	}

	IntBuffer* IntBuffer::allocate(int size) {
		return new IntBuffer(size);
	}

	IntBuffer* IntBuffer::allocate(std::vector<unsigned int> vec) {
		IntBuffer* buff = IntBuffer::allocate(vec.size());
		for (unsigned int& v : vec) {
			buff->put(v);
		}
		return buff;
	}

	void IntBuffer::clear() {
		memset(_data, 0, _size * sizeof(float));
		rewind();
	}

	IntBuffer* IntBuffer::put(int i) {
		int index = _nextIndex++;
		if (index > _size - 1) {
			std::cerr << "Buffer overflow exception ! (" << index << ")" << std::endl;
			return this;
		}
		_data[index] = i;
		return this;
	}

	IntBuffer* IntBuffer::put(int index, int i) {
		if (index > _size - 1) {
			std::cerr << "Buffer overflow exception ! (" << index << ")" << std::endl;
			return this;
		}
		_data[index] = i;
		return this;
	}

	IntBuffer* IntBuffer::put(int* data, int size) {
		for (int i = 0; i < size; i++) {
			put(data[i]);
		}
		return this;
	}

	int IntBuffer::get() {
		return _data[_nextIndex];
	}

	int IntBuffer::get(int index) {
		return _data[index];
	}

	int* IntBuffer::getData() {
		return _data;
	}
	void IntBuffer::print(int c) {
		for (int i = 0; i < size() / c; i++) {
			for (int a = 0; a < c; a++) {
				std::cout << get(i * c + a) << ",";
			}
			std::cout << std::endl;
		}
	}
}