#ifndef BUFFERS_H

#include "Engine/Utils/Buffers/FloatBuffer.h"
#include "Engine/Utils/Buffers/IntBuffer.h"
#include "glm/glm.hpp"

namespace NovaEngine {
	FloatBuffer* createFloatBuffer(float* data, int size);
	IntBuffer* createIntBuffer(int* data, int size);

	void populateFromBuffer(glm::vec3& vector, FloatBuffer* buf, int index);
	void populateFromBuffer2(glm::vec2& vector, FloatBuffer* buf, int index);
}

#endif // !BUFFERS_H