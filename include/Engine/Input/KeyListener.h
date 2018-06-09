#ifndef KEY_LISTENER_H
#define KEY_LISTENER_H

#include "Engine/Input/Input.h"

namespace NovaEngine {
	class KeyListener {
	public:
		virtual void onKeyEvent(KeyEvent e);
	};
}

#endif // !KEY_LISTENER_H