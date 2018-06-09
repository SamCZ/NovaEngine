#ifndef UI_BUTTON_H
#define UI_BUTTON_H

#include <string>
#include "Engine/UI/UINode.h"
#include "Engine/UI/Renderers/RectangleRenderer.h"
#include "Engine/UI/Renderers/TextRenderer.h"

namespace NovaEngine {
	class Button : public UINode {
	private:
		RectangleRendeer* _bgRenderer;
		TextRendeer* _textRenderer;
	public:
		Button(std::string text);
		Button(std::string text, int width);

		void setText(std::string text);
		std::string getText() const;
	};
}

#endif // !UI_BUTTON_H