#ifndef FIRST_PERSON_CONTROL_H
#define FIRST_PERSON_CONTROL_H

#include <iostream>
#include "Engine/Input/InputManager.h"
#include "SDL.h"
#include "glm/glm.hpp"
#include "Engine/Utils/Utils.h"

namespace NovaEngine {
	class FirstPersonControl : public MouseListener, public KeyListener {
	public:

		bool _enabled = true;
		bool _flyMode = false;
		float lastBounceY = 0;
		float lastRotBounce = 0;

		inline FirstPersonControl(Camera* camera) {
			_camera = camera;
			_W = false;
			_S = false;
			_A = false;
			_D = false;
		}

		inline void setCamera(Camera* camera) {
			_camera = camera;
		}

		inline void onKeyEvent(KeyEvent e) override {
			if (e.isText) return;

			if (e.keyCode == SDLK_w) {
				_W = e.state;
			}
			if (e.keyCode == SDLK_s) {
				_S = e.state;
			}
			if (e.keyCode == SDLK_a) {
				_A = e.state;
			}
			if (e.keyCode == SDLK_d) {
				_D = e.state;
			}
			if (e.keyCode == SDLK_LSHIFT) {
				_LS = e.state;
			}
		}

		inline void update(float delta) {
			if(!_enabled) return;
			float movementSpeed = 0.1f;
			if (_LS) {
				movementSpeed = 1.5f;
			}
			delta *= movementSpeed;
			if (_flyMode) {
				delta *= _flyBoost;
			}
			float sinDelta = delta * 2.0f;
			if (!_flyMode) {
				delta *= 0.1f;
			}

			/*if (_W && !_flyMode) {
				_camera->getLocation().y -= lastBounceY;
				_camera->getRotation().z = 0;
			}*/

			if (_W) {
				glm::vec3& loc = _camera->getLocation();
				glm::vec3& rot = _camera->getRotation();

				if (_flyMode) {
					loc.x -= -glm::cos(glm::radians(rot.x)) * -glm::sin(glm::radians(rot.y)) * delta;
					loc.y -= -glm::sin(glm::radians(rot.x)) * delta;
					loc.z -= -glm::cos(glm::radians(rot.x)) * -glm::cos(glm::radians(rot.y)) * delta;
				} else {
					loc.x += -glm::sin(glm::radians(rot.y)) * delta;
					loc.z += -glm::cos(glm::radians(rot.y)) * delta;
				}
			}
			if (_S) {
				glm::vec3& loc = _camera->getLocation();
				glm::vec3& rot = _camera->getRotation();

				if (_flyMode) {
					loc.x += -glm::cos(glm::radians(rot.x)) * -glm::sin(glm::radians(rot.y)) * delta;
					loc.y += -glm::sin(glm::radians(rot.x)) * delta;
					loc.z += -glm::cos(glm::radians(rot.x)) * -glm::cos(glm::radians(rot.y)) * delta;
				} else {
					loc.x -= -glm::sin(glm::radians(rot.y)) * delta;
					loc.z -= -glm::cos(glm::radians(rot.y)) * delta;
				}
			}

			if (_A) {
				glm::vec3& loc = _camera->getLocation();
				glm::vec3& rot = _camera->getRotation();

				loc.x += -glm::sin(glm::radians(rot.y + 90)) * delta;
				loc.z += -glm::sin(glm::radians(-rot.y)) * delta;
			}

			if (_D) {
				glm::vec3& loc = _camera->getLocation();
				glm::vec3& rot = _camera->getRotation();

				loc.x -= -glm::sin(glm::radians(rot.y + 90)) * delta;
				loc.z -= -glm::sin(glm::radians(-rot.y)) * delta;
			}

			/*if (_W && !_flyMode) {
				static float atime = 0;
				atime += sinDelta * 1.5f;
				lastBounceY = lerp(-0.001f, 0.005f, glm::cos(atime) / 2.0f + 0.5f) * 0.5f;
				_camera->getLocation().y += lastBounceY;

				lastRotBounce = lerp(-0.5f, 0.5f, glm::cos(atime * 0.55f) / 2.0f + 0.5f);
				_camera->getRotation().z += lastRotBounce;
			}*/
		}

		inline void onMouseMove(int x, int y, int dx, int dy) override {
			if (!_enabled) return;
			glm::vec3& rot = _camera->getRotation();
			rot.x -= dy * 0.15f;
			rot.y -= dx * 0.15f;

			
			if (rot.y >= 360.0f || rot.y <= -360.0f) {
				rot.y = glm::mod(rot.y, 360.0f);
			}

			if (rot.x <= -90.0f) {
				rot.x = -90.0f;
			} else if (rot.x >= 90.0f) {
				rot.x = 90.0f;
			}
		}

		inline void onMouseWheel(MouseWheel wheel) override {
			if (!_enabled) return;
			if (wheel != MouseWheel::Up) {
				_flyBoost += 2.0;
			} else {
				_flyBoost -= 2.0;
			}
			if (_flyBoost < 0) {
				_flyBoost = 0;
			}
		}

	private:
		Camera* _camera;
		bool _W;
		bool _S;
		bool _A;
		bool _D;
		bool _LS;
		float _flyBoost = 1.0;
	};

}

#endif // !FIRST_PERSON_CONTROL_H