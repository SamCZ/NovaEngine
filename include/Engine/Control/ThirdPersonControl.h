#ifndef THIRD_PERSON_CONTROL_H
#define THIRD_PERSON_CONTROL_H

#include <iostream>
#include "Engine/Input/InputManager.h"
#include "glm/glm.hpp"
#include "Engine/Scene/Camera.h"

namespace NovaEngine {
	class ThirdPersonControl : public MouseListener, public KeyListener {
	private:
		Camera* _camera;
		bool _enabled;
		bool _dragged;
		bool _cameraRelocatte = false;
		glm::vec3 _center;
		float _distanceFromCenter = 5;
		float _angleAroundCenter;
		float _pitch;
		float _defaultYaw;
		glm::vec2 mPos1;
	public:
		inline ThirdPersonControl(Camera* camera) {
			_camera = camera;
		}

		inline void update() {
			if(!_enabled) return;
			float horizonDistance = this->getHorizontalDistance();
			float vertDistance = this->getVerticalDistance();
			float theta = _defaultYaw + _angleAroundCenter;
			float offsetX = horizonDistance * glm::sin(glm::radians(theta));
			float offsetZ = horizonDistance * glm::cos(glm::radians(theta));

			float xAngle = 180 + _defaultYaw + _angleAroundCenter;
			float yAngle = std::fmod(-_pitch, 360.0f);

			_camera->setLocation(_center.x - offsetX, _center.y + vertDistance, _center.z - offsetZ);
			_camera->setRotation(yAngle, xAngle, 0);
		}

		inline float getHorizontalDistance() {
			return _distanceFromCenter * glm::cos(glm::radians(_pitch));
		}

		inline float getVerticalDistance() {
			return _distanceFromCenter * glm::sin(glm::radians(_pitch));
		}


		inline void onMouseMove(int x, int y, int dx, int dy) override {
			if (!_enabled) return;
			if (_dragged) {
				_angleAroundCenter -= dx * 0.1f;
				_pitch += dy * 0.1f;
			}
			if (_cameraRelocatte) {
				glm::vec3 click3d1 = _camera->getWorldPosition(mPos1.x, _camera->getHeight() - mPos1.y, 0.0f);
				glm::vec3 click3d2 = _camera->getWorldPosition((float)x, (float)_camera->getHeight() - (float)y, 0.0f);
				_center += click3d1 - click3d2;
				mPos1 = glm::vec2(x, y);
			}
		}

		inline void onMouseEvent(ButtonEvent e) override {
			if (!e.state) _dragged = false;
			if (!_enabled) return;
			if (e.type == ButtonType::Left) {
				_dragged = e.state;
			} else if (e.type == ButtonType::Right) {
				_cameraRelocatte = e.state;
				if (_cameraRelocatte) {
					mPos1 = glm::vec2(e.x, e.y);
				} else {
					//this.moveToCenter = false;
				}
			}
		}
		inline void onMouseWheel(MouseWheel e) override {
			if (!_enabled) return;
			if (e == MouseWheel::Down) {
				_distanceFromCenter -= 0.1f;
			} else {
				_distanceFromCenter += 0.1f;
			}
			if (_distanceFromCenter < 0) {
				_distanceFromCenter = 0;
			}
		}

		inline void setEnabled(bool state) {
			_enabled = state;
		}

	};
}

#endif // !THIRD_PERSON_CONTROL_H