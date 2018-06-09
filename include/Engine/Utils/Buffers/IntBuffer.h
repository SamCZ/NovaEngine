#ifndef INT_BUFFER_H
#define INT_BUFFER_H

#include "Engine/Utils/Buffers/Buffer.h"
#include <vector>

namespace NovaEngine {
	class IntBuffer : public Buffer {
	public:
		~IntBuffer();
		static IntBuffer* allocate(int size);
		static IntBuffer* allocate(std::vector<unsigned int> vec);

		inline int size() override {
			return _size;
		}
		void clear() override;

		inline BufferType getType() override {
			return BufferType::Int;
		}

		IntBuffer* put(int i);
		IntBuffer* put(int index, int i);
		IntBuffer* put(int* data, int size);

		int get();
		int get(int index);

		int* getData();

		void print(int c);
	private:
		IntBuffer(int size);
		int _size;
		int* _data;
	};
}

#endif // !INT_BUFFER_H