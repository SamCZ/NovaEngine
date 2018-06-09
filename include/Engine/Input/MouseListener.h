#ifndef MOUSE_LISTENER_H
#define MOUSE_LISTENER_H

#include "Engine/Input/Input.h"

namespace NovaEngine {
	class MouseListener {
	public:
		virtual void onMouseMove(int x, int y, int dx, int dy);
		virtual void onMouseEvent(ButtonEvent e);
		virtual void onMouseWheel(MouseWheel wheel);
	};
}

#endif // !MOUSE_LISTENER_H