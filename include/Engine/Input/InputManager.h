#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include <vector>

#include "SDL.h"
#include "glm/glm.hpp"

#include "Engine/Input/Input.h"
#include "Engine/Input/MouseListener.h"
#include "Engine/Input/KeyListener.h"

namespace NovaEngine {
	class InputManager {
	public:
		InputManager();
		~InputManager();

		void addMouseListener(MouseListener* listener);
		void removeMouseListener(MouseListener* listener);

		void addKeyListener(KeyListener* listener);
		void removeKeyListener(KeyListener* listener);

		void setMouseLocation(int x, int y);
		glm::vec2 getMouseLocation() const;
		int getMouseX() const;
		int getMouseY() const;
		int getMouseDX() const;
		int getMouseDY() const;

		void processEvent(SDL_Event& e);
	private:
		std::vector<MouseListener*> _mouseListeners;
		std::vector<KeyListener*> _keyListeners;
		int _mouseX;
		int _mouseY;
		int _dx;
		int _dy;
	};
}

#endif // !INPUT_MANAGER_H