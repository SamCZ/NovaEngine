#ifndef SCROLL_PANE_RENDERER_H
#define SCROLL_PANE_RENDERER_H


#include "Engine/UI/TypeRenderer.h"

#include "Engine/Input/InputManager.h"
#include "Engine/Utils/Files.h"
#include "Engine/UI/Types/ScrollPane.h"

namespace NovaEngine {
	class TextRendeer : public TypeRenderer {
	private:

	public:
		int fontSize = 13;
		int paddingX = 0;
		int paddingY = 0;

		std::string text;
		bool updtWidth;
		bool updtHeight;
		bool updateSize;

		inline TextRendeer() : text() {
			
		}

		inline void render(UIRenderer* renderer, UINode* node) override {
			if (updateSize) {
				int ty;
				int tx = renderer->getTextSize(text, fontSize, &ty);
				if (updtWidth) {
					node->setWidth(tx + paddingX * 2);
				}
				if (updtHeight) {
					node->setHeight(ty + paddingY * 2);
				}
				updateSize = false;
			}
			renderer->drawText(text, node->getWorldX() + node->getWidth() / 2, node->getWorldY() + node->getHeight() / 2, fontSize, makeRGB(0, 0, 0), Align::Center, Align::Center);
		}
	};
}

#endif // !SCROLL_PANE_RENDERER_H