#ifndef INPUT_TYPES_H
#define INPUT_TYPES_H

namespace NovaEngine {

	enum class ButtonType {
		Left, Middle, Right
	};

	enum class MouseWheel {
		Down, Up
	};

	struct ButtonEvent {
		ButtonType type;
		int x;
		int y;
		bool state;
		bool doubleClick;
	};

	struct KeyEvent {
		bool isText;
		char character;
		int keyCode;
		int mod;
		bool state;
	};

}

#endif // !INPUT_TYPES_H