#include "Engine/Input/InputManager.h"

namespace NovaEngine {
	InputManager::InputManager() {
		_mouseX = -1;
		_mouseY = -1;
		_dx = 0;
		_dy = 0;
	}

	InputManager::~InputManager() {
		_mouseListeners.clear();
		_keyListeners.clear();
	}

	void InputManager::addMouseListener(MouseListener * listener) {
		_mouseListeners.push_back(listener);
	}

	void InputManager::removeMouseListener(MouseListener * listener) {
		_mouseListeners.erase(std::find(_mouseListeners.begin(), _mouseListeners.end(), listener));
	}

	void InputManager::addKeyListener(KeyListener* listener) {
		_keyListeners.push_back(listener);
	}

	void InputManager::removeKeyListener(KeyListener* listener) {
		_keyListeners.erase(std::find(_keyListeners.begin(), _keyListeners.end(), listener));
	}

	void InputManager::setMouseLocation(int x, int y) {
		if (_mouseX != -1 && _mouseY != -1) {
			_dx = x - _mouseX;
			_dy = y - _mouseY;
		}
		_mouseX = x;
		_mouseY = y;
	}

	glm::vec2 InputManager::getMouseLocation() const {
		return glm::vec2(_mouseX, _mouseY);
	}

	int InputManager::getMouseX() const {
		return _mouseX;
	}

	int InputManager::getMouseY() const {
		return _mouseY;
	}

	int InputManager::getMouseDX() const {
		return _dx;
	}

	int InputManager::getMouseDY() const {
		return _dy;
	}

	void InputManager::processEvent(SDL_Event& e) {
		if (e.type == SDL_MOUSEMOTION) {
			for (MouseListener* lis : _mouseListeners) {
				lis->onMouseMove(e.motion.x, e.motion.y, e.motion.xrel, e.motion.yrel);
			}
		} else if (e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP) {
			ButtonEvent evt;
			if (e.button.button == SDL_BUTTON_LEFT) {
				evt.type = ButtonType::Left;
			} else if (e.button.button == SDL_BUTTON_MIDDLE) {
				evt.type = ButtonType::Middle;
			} else if (e.button.button == SDL_BUTTON_RIGHT) {
				evt.type = ButtonType::Right;
			}
			evt.x = e.button.x;
			evt.y = e.button.y;
			evt.state = e.button.state == SDL_PRESSED ? true : false;
			evt.doubleClick = false;
			if (e.button.clicks == 2) {
				evt.doubleClick = true;
			}
			for (MouseListener* lis : _mouseListeners) {
				lis->onMouseEvent(evt);
			}
		} else if (e.type == SDL_MOUSEWHEEL) {
			MouseWheel wheel;
			if (e.wheel.y == -1) {
				wheel = MouseWheel::Up;
			} else {
				wheel = MouseWheel::Down;
			}
			for (MouseListener* lis : _mouseListeners) {
				lis->onMouseWheel(wheel);
			}
		} else if (e.type == SDL_TEXTINPUT) {
			char* text = e.text.text;
			int textLen = strlen(text);
			if (textLen > 0) {
				KeyEvent evt;
				evt.isText = true;
				evt.character = text[textLen - 1];
				evt.keyCode = (int)text[textLen - 1];
				evt.state = true;
				for (KeyListener* lis : _keyListeners) {
					lis->onKeyEvent(evt);
				}
			}
		} else if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP) {
			KeyEvent evt;
			evt.isText = false;
			evt.keyCode = e.key.keysym.sym;
			evt.state = e.key.state == SDL_PRESSED ? true : false;
			evt.mod = e.key.keysym.mod;
			for (KeyListener* lis : _keyListeners) {
				lis->onKeyEvent(evt);
			}
		}
	}
}