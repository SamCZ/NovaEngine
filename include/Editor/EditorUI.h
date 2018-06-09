#ifndef EDITOR_UI_H
#define EDITOR_UI_H

#include "Engine/NovaEngine.h"

namespace NovaEngine {
	class EditorUI {
	private:

		void drawMaterialWindow();
	public:
		Material* _testMat;

		EditorUI();
		~EditorUI();

		void renderImGui();
	};
}

#endif // !EDITOR_UI_H