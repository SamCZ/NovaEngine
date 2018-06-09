#include "Engine/Utils/NativeObject.h"

namespace NovaEngine {
	bool NativeObject::isNeedsUpdate() const {
		return _needsUpdate;
	}
	void NativeObject::markClean() {
		_needsUpdate = false;
	}
	void NativeObject::markUpdate() {
		_needsUpdate = true;
	}

	GLuint NativeObject::getId() {
		return _id;
	}

	void NativeObject::setId(GLuint id) {
		_id = id;
	}
}