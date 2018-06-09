#ifndef VERTEX_BUFFER_H
#define VERTEX_BUFFER_H

#include "Engine/Utils/NativeObject.h"
#include "Engine/Utils/Buffers/Buffer.h"

namespace NovaEngine {
	namespace MeshBuffer {
		enum class Type {
			Position,
			Normal,
			TexCoord,
			Color,
			Tangent,
			BiTangent,
			Index,
			InstanceData
		};

		enum class Usage {
			Static,
			Dynamic,
			Stream,
			CpuOnly
		};

		enum class Format {
			Float,
			Int
		};
	}

	class VertexBuffer : public NativeObject {
	public:
		VertexBuffer(MeshBuffer::Type type);
		~VertexBuffer();

		void setupData(MeshBuffer::Usage usage, int components, MeshBuffer::Format format, Buffer* buffer);
		void updateData(Buffer* buffer);

		void setUsage(MeshBuffer::Usage usage);

		MeshBuffer::Type getType() const;
		MeshBuffer::Usage getUsage() const;
		MeshBuffer::Format getFormat() const;

		int getNumComponents() const;
		Buffer* getBuffer();
	private:
		MeshBuffer::Type _type;
		MeshBuffer::Usage _usage;
		MeshBuffer::Format _format;
		int _components;
		Buffer* _buffer;
	};
}

#endif // !VERTEX_BUFFER_H