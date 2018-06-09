#include "Engine/UI/Types/Button.h"

namespace NovaEngine {

	Button::Button(std::string text) {
		_bgRenderer = new RectangleRendeer();

		Border border = makeBorder(1.0f, makeHEX("616161"), makeRadius(2.0f));
		_bgRenderer->defaultStyle.border = border;
		_bgRenderer->defaultStyle.background = makeLinearGradientPaint(true, makeHEX("FCFCFC"), makeHEX("C2C0C1"));
		_bgRenderer->focusedStyle = RectStyle(_bgRenderer->defaultStyle);
		_bgRenderer->clickedStyle = RectStyle(_bgRenderer->defaultStyle);
		_bgRenderer->clickedStyle.background = makeLinearGradientPaint(true, makeHEX("C2C0C1"), makeHEX("FCFCFC"));

		addRenderer(_bgRenderer, 0);


		_textRenderer = new TextRendeer();
		addRenderer(_textRenderer, 1);
		_textRenderer->text = text;

		_textRenderer->updtWidth = true;
		_textRenderer->updtHeight = true;
		_textRenderer->updateSize = true;

		_textRenderer->paddingX = 5;
		_textRenderer->paddingY = 2;
	}

	Button::Button(std::string text, int width) : Button(text) {
		setSize(width, 0);
		_textRenderer->updtWidth = false;
	}

	void Button::setText(std::string text) {
		_textRenderer->text = text;
		_textRenderer->updateSize = true;
	}

	std::string Button::getText() const {
		return _textRenderer->text;
	}
}