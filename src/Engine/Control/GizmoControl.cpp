#include "Engine/Control/GizmoControl.h"
#include "LibGizmo/IGizmo.h"
#include "glm/gtx/matrix_decompose.hpp"

namespace NovaEngine {
	int GizmoControl::getSelectedModeIndex() const {
		return _mode == GizmoMode::Move ? 0 : (_mode == GizmoMode::Rotate ? 1 : (_mode == GizmoMode::Scale ? 2 : 0));
	}

	GizmoControl::GizmoControl(ViewPort* wp, InputManager* im) {
		im->addKeyListener(this);
		im->addMouseListener(this);
		_wp = wp;
		_camera = wp->getCamera();

		_gizmos[0] = CreateMoveGizmo();
		_gizmos[1] = CreateRotateGizmo();
		_gizmos[2] = CreateScaleGizmo();

		_controlSpatial = nullptr;
		_mode = GizmoMode::Move;
		_capturingMouse = false;
	}

	void GizmoControl::setControl(Spatial* spatial) {
		_controlSpatial = spatial;
	}

	void GizmoControl::draw() {
		if(_controlSpatial == nullptr) return;
		IGizmo* gizmo = _gizmos[getSelectedModeIndex()];
		_editMat = _controlSpatial->getModelMatrix();

		gizmo->SetEditMatrix(&_editMat[0][0]);
		gizmo->SetScreenDimension(_camera->getWidth(), _camera->getHeight());
		gizmo->SetCameraMatrix(&_camera->getViewMatrix()[0][0], &_camera->getProjectionMatrix()[0][0]);

		glm::vec3 wloc = _controlSpatial->getWorldLocation();
		gizmo->Draw(wloc.x, wloc.y, wloc.z);
	}

	void GizmoControl::setCamera(Camera* camera) {
		_camera = camera;
	}

	void GizmoControl::setMode(const GizmoMode& mode) {
		_mode = mode;
	}

	bool GizmoControl::isCapturing(int x, int y) const {
		return _gizmos[getSelectedModeIndex()]->OnMouseDown(x, y);
	}

	void GizmoControl::setEnabled(bool enabled) {
		_enabled = enabled;
	}

	void GizmoControl::onMouseMove(int x, int y, int dx, int dy) {
		if (_controlSpatial == nullptr) return;
		if(!_enabled) return;
		_gizmos[getSelectedModeIndex()]->OnMouseMove(x, y);
		if (!_capturingMouse) return;
		if (_editMat != _controlSpatial->getModelMatrix()) {
			glm::vec3 scale;
			glm::quat rotation;
			glm::vec3 translation;
			glm::vec3 skew;
			glm::vec4 perspective;
			glm::decompose(_editMat, scale, rotation, translation, skew, perspective);

			if (_mode == GizmoMode::Move) {
				_controlSpatial->setLocation(translation - _controlSpatial->getParentLocation());
			}
			if (_mode == GizmoMode::Rotate) {
				glm::vec3 angles = glm::eulerAngles(rotation) * 57.295776f;
				_controlSpatial->setRotation(angles);
			}
			if (_mode == GizmoMode::Scale) {
				_controlSpatial->setScale(scale);
			}

			_editMat = _controlSpatial->getModelMatrix();
		}
	}

	void GizmoControl::onMouseEvent(ButtonEvent e) {
		if (_controlSpatial == nullptr) return;
		if (!_enabled) return;
		if (e.type == ButtonType::Left) {
			if (e.state) {
				_capturingMouse = _gizmos[getSelectedModeIndex()]->OnMouseDown(e.x, e.y);
			} else {
				_gizmos[getSelectedModeIndex()]->OnMouseUp(e.x, e.y);
				_capturingMouse = false;
			}
		}
	}

	void GizmoControl::onKeyEvent(KeyEvent e) {
		if (_controlSpatial == nullptr) return;
		if(e.isText) return;
		if(!e.state) return;

		if (e.keyCode == SDLK_e) {
			_mode = GizmoMode::Move;
		} else if (e.keyCode == SDLK_r) {
			_mode = GizmoMode::Rotate;
		} else if (e.keyCode == SDLK_t) {
			_mode = GizmoMode::Scale;
		}
	}
}