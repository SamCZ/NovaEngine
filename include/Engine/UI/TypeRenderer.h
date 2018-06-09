#ifndef TYPE_RENDERER_H
#define TYPE_RENDERER_H

#include "Engine/UI/UIRenderer.h"
#include "Engine/UI/UINode.h"

namespace NovaEngine {
	class TypeRenderer {
	public:
		int R_INDEX = 0;

		virtual void render(UIRenderer* renderer, UINode* node) = 0;
	};
}

#endif // !TYPE_RENDERER_H