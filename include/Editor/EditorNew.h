#ifndef EDITOR_NEW_H
#define EDITOR_NEW_H

#include "Engine/NovaEngine.h"
#include "Editor/EditorUI.h"

using namespace NovaEngine;

class EditorNew : public Engine {
private:
	EditorUI* _editorUI;
public:
	~EditorNew() {

	}

	EditorNew() { }

	void init() override {
		_editorUI = new EditorUI();

		_firstPersonControl->_enabled = false;

		Material* mat = _spriteBatch->getMaterial();

		_editorUI->_testMat = mat;

		GameObject* obj = new GameObject();
		obj->setMesh(new Box(0.5, 0.5, 0.5));
		obj->setMaterial(mat);
		obj->setLocation(0, 0, -5);

		_mainScene->addChild(obj);
	}

	void update() override {

	}

	void render() override {
		_renderer->clearScreen(true, true, false);
		_renderer->clearColor(makeRGB(0, 0, 0));
		_renderManager->renderViewPort(_viewPort);
	}

	void renderUI() override {

	}

	void renderImGui() override {
		_editorUI->renderImGui();
	}

	void onMouseMove(int x, int y, int dx, int dy) override {

	}

	void onMouseEvent(ButtonEvent e) override {

	}

	void onKeyEvent(KeyEvent e) override {

	}

};

#endif // !EDITOR_NEW_H