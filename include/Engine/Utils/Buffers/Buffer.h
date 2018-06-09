#ifndef BUFFER_H
#define BUFFER_H

namespace NovaEngine {
	enum class BufferType {
		Float,
		Int,
		Byte,
		Short
	};

	class Buffer {
	public:
		virtual inline ~Buffer() {  }
		virtual int size() = 0;
		virtual void clear() = 0;
		virtual BufferType getType() = 0;

		inline void rewind() {
			_nextIndex = 0;
		}

		inline void position(int pos) {
			_nextIndex = pos;
		}

	protected:
		int _nextIndex = 0;
	};
}

#endif // !BUFFER_H