#ifndef GIZMO_CONTROL_H
#define GIZMO_CONTROL_H

#include "Engine/Input/InputManager.h"
#include "Engine/Scene/ViewPort.h"
#include "Engine/Scene/Spatial.h"

class IGizmo;

namespace NovaEngine {

	enum class GizmoMode {
		Move, Rotate, Scale
	};

	class GizmoControl : public MouseListener, public KeyListener {
	private:
		IGizmo* _gizmos[3];
		Spatial* _controlSpatial;
		ViewPort* _wp;
		Camera* _camera;
		GizmoMode _mode;
		glm::mat4 _editMat;
		bool _capturingMouse;
		bool _enabled;

		int getSelectedModeIndex() const;
	public:
		GizmoControl(ViewPort* wp, InputManager* im);

		void setControl(Spatial* spatial);
		void draw();
		void setMode(const GizmoMode& mode);
		bool isCapturing(int x, int y) const;
		void setEnabled(bool enabled);
		void setCamera(Camera* camera);

		void onMouseMove(int x, int y, int dx, int dy) override;
		void onMouseEvent(ButtonEvent e) override;
		void onKeyEvent(KeyEvent e) override;
	};
}

#endif // !GIZMO_CONTROL_H