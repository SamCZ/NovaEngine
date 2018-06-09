#ifndef NATIVE_OBJECT_H
#define NATIVE_OBJECT_H

#include "GL/glew.h"

namespace NovaEngine {
	class NativeObject {
	public:
		bool isNeedsUpdate() const;
		void markClean();
		void markUpdate();

		GLuint getId();
		void setId(GLuint id);
	protected:
		bool _needsUpdate = true;
		GLuint _id = -1;
	};
}

#endif // !NATIVE_OBJECT_H