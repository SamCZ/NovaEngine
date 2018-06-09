#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include "Engine/UI/UIRenderer.h"
#include "Engine/UI/UI.h"
#include "Engine/Input/InputManager.h"

namespace NovaEngine {
	class UIManager : public MouseListener, public KeyListener {
	private:
		UIRenderer* _uiRenderer;
		InputManager* _inputManager;
		UINode* _rootNode;

		bool _focusMoveFound;
		int _mx;
		int _my;
		UINode* _focusedItem;

		void renderNode(UINode* node);

	public:
		UIManager(UIRenderer* uiRenderer, InputManager* inputManager, UINode* rootNode);
		~UIManager();

		void updateAndRender();

		void onMouseMove(int x, int y, int dx, int dy) override;
		void onMouseEvent(ButtonEvent e) override;
		void onKeyEvent(KeyEvent e) override;
		void onMouseWheel(MouseWheel e) override;

		void onMouseMotion(int mx, int my, int dx, int dy, UINode* node);
		void onMouseWheelEvent(MouseWheel e, UINode* node);
		void onMouseEvent(ButtonEvent e, UINode* node);
		void onKeyEvent(KeyEvent e, UINode* node);
	};
}

#endif // !UI_MANAGER_H