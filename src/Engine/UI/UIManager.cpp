#include "Engine/UI/UIManager.h"
#include <iostream>

namespace NovaEngine {
	UIManager::UIManager(UIRenderer* uiRenderer, InputManager* inputManager, UINode* rootNode) {
		_uiRenderer = uiRenderer;
		_inputManager = inputManager;
		_rootNode = rootNode;
		_focusedItem = nullptr;
	}

	UIManager::~UIManager() {
		delete _uiRenderer;
	}

	void UIManager::renderNode(UINode* node) {
		int clipId = -1;
		if (node->isClipEnabled()) {
			clipId = _uiRenderer->beginClip(node->getWorldX(), node->getWorldY(), node->getClip().width, node->getClip().height);
		}
		for (TypeRenderer* renderer : node->getRenderers()) {
			if (node->isVisible()) {
				renderer->render(_uiRenderer, node);
			}
		}
		for (UINode* child : node->getChilds()) {
			renderNode(child);
		}
		if (node->isClipEnabled()) {
			_uiRenderer->endClip(clipId);
		}
	}

	void UIManager::updateAndRender() {
		renderNode(_rootNode);
		_uiRenderer->clearClipping();
	}

	void UIManager::onMouseMotion(int mx, int my, int dx, int dy, UINode* node) {
		if (node == nullptr) return;
		int childCount = node->getChilds().size();
		for (int i = 0; i < childCount; i++) {
			UINode* subNode = node->getChilds()[childCount - 1 - i];
			onMouseMotion(mx, my, dx, dy, subNode);
		}

		if (!_focusMoveFound) {
			node->onMouseMove(mx, my, dx, dy);
			if (node->isFocused()) {
				_focusedItem = node;
				_focusMoveFound = true;
				//continue;
			}
		} else {
			node->setFocused(false);
		}

		//node->onMouseMotion(mx, my);
	}

	void UIManager::onMouseWheelEvent(MouseWheel e, UINode* node) {
		if (node == nullptr) return;
		int childCount = node->getChilds().size();
		for (int i = 0; i < childCount; i++) {
			UINode* subNode = node->getChilds()[childCount - 1 - i];
			onMouseWheelEvent(e, subNode);
		}
		node->onMouseWheel(e);
	}

	void UIManager::onMouseEvent(ButtonEvent e, UINode* node) {
		if (node == nullptr) return;
		int childCount = node->getChilds().size();
		for (int i = 0; i < childCount; i++) {
			UINode* subNode = node->getChilds()[childCount - 1 - i];
			onMouseEvent(e, subNode);
		}
		node->onMouseEvent(e);
	}

	void UIManager::onKeyEvent(KeyEvent e, UINode* node) {
		if (node == nullptr) return;
		int childCount = node->getChilds().size();
		for (int i = 0; i < childCount; i++) {
			UINode* subNode = node->getChilds()[childCount - 1 - i];
			onKeyEvent(e, subNode);
		}
		node->onKeyEvent(e);
	}

	void UIManager::onMouseMove(int x, int y, int dx, int dy) {
		_focusMoveFound = false;
		_focusedItem = nullptr;
		onMouseMotion(x, y, dx, dy, _rootNode);
		_mx = x;
		_my = y;
	}

	void UIManager::onMouseEvent(ButtonEvent e) {
		onMouseEvent(e, _rootNode);
	}

	void UIManager::onKeyEvent(KeyEvent e) {
		onKeyEvent(e, _rootNode);
	}

	void UIManager::onMouseWheel(MouseWheel e) {
		onMouseWheelEvent(e, _rootNode);
	}
}