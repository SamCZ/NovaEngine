#ifndef FLOAT_BUFFER_H
#define FLOAT_BUFFER_H

#include "Engine/Utils/Buffers/Buffer.h"
#include <vector>
#include "glm/glm.hpp"

namespace NovaEngine {
	class FloatBuffer : public Buffer {
	public:
		~FloatBuffer();
		static FloatBuffer* allocate(int size);
		static FloatBuffer* allocate3(std::vector<glm::vec3> vec);
		static FloatBuffer* allocate2(std::vector<glm::vec2> vec);

		inline int size() override {
			return _size;
		}
		void clear() override;

		inline BufferType getType() override {
			return BufferType::Float;
		}

		FloatBuffer* put(float f);
		FloatBuffer* put(int index, float f);

		FloatBuffer* put(float* data, int size);

		//void put(float* data, int size);

		float get();
		float get(int index);

		float* getData();

		void print(int c);
	private:
		FloatBuffer(int size);
		int _size;
		float* _data;
	};
}

#endif // !FLOAT_BUFFER_H