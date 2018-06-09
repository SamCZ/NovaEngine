#include "Editor/EditorUI.h"

#include "ImGui/imgui_helper.h"

namespace NovaEngine {
	EditorUI::EditorUI() {
	}

	EditorUI::~EditorUI() {
		//TODO: Save data
	}
	void EditorUI::renderImGui() {
		BeginIds();

		drawMaterialWindow();
	}

	void EditorUI::drawMaterialWindow() {
		Material* mat = _testMat;
		TechniqueDef* technique = mat->getTechnique("Default");

		

		ImGui::ShowTestWindow();
	}
}