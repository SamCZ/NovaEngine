#include "Engine/Utils/Buffers.h"

namespace NovaEngine {
	FloatBuffer* createFloatBuffer(float * data, int size) {
		FloatBuffer* fb = FloatBuffer::allocate(size);
		fb->put(data, size);
		delete[] data;
		return fb;
	}
	IntBuffer* createIntBuffer(int * data, int size) {
		IntBuffer* ib = IntBuffer::allocate(size);
		ib->put(data, size);
		delete[] data;
		return ib;
	}

	void populateFromBuffer(glm::vec3& vector, FloatBuffer* buf, int index) {
		vector.x = buf->get(index * 3);
		vector.y = buf->get(index * 3 + 1);
		vector.z = buf->get(index * 3 + 2);
	}

	void populateFromBuffer2(glm::vec2& vector, FloatBuffer* buf, int index) {
		vector.x = buf->get(index * 2);
		vector.y = buf->get(index * 2 + 1);
	}
}