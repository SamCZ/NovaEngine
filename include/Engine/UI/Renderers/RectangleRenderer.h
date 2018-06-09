#ifndef RECTANGLE_RENDERER_H
#define RECTANGLE_RENDERER_H

#include "Engine/UI/TypeRenderer.h"

#include "Engine/Input/InputManager.h"
#include "Engine/Utils/Files.h"

namespace NovaEngine {
	struct RectStyle {
		Paint background;
		Border border;
		DropShadow shadow;
	};

	class RectangleRendeer : public TypeRenderer {
	public:
		RectStyle defaultStyle;
		RectStyle focusedStyle;
		RectStyle clickedStyle;

		inline RectangleRendeer() {
			defaultStyle.background = makeColorPaint(makeHEX("D9D9D9"));
			defaultStyle.border = makeBorder(1.0f, makeHEX("959595"), makeRadius(0.0f));
			defaultStyle.shadow = makeShadow(0, makeRGB(0, 0, 0));

			focusedStyle = RectStyle(defaultStyle);
			clickedStyle = RectStyle(defaultStyle);
		}

		inline void render(UIRenderer* renderer, UINode* node) override {
			RectStyle style = defaultStyle;
			if (node->isClicked()) {
				style = clickedStyle;
			} else if (node->isFocused()) {
				style = focusedStyle;
			}
			renderer->drawRect(node->getWorldX(), node->getWorldY(), node->getWidth(), node->getHeight(), style.background, style.border, style.shadow);
		}
	};
}

#endif // !RECTANGLE_RENDERER_H