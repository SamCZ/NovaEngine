#ifndef NOVA_ENGINE_H
#define NOVA_ENGINE_H

#include <iostream>

#include "GL/glew.h"

#include "Engine/Render/Display.h"
#include "Engine/Input/InputManager.h"
#include "Engine/Scene/Camera.h"
#include "Engine/Render/Timer.h"
#include "Engine/Loaders/AssetManager.h"
#include "Engine/Render/RenderManager.h"
#include "Engine/Scene/ViewPort.h"
#include "Engine/Render/SpriteBatch.h"
#include "Engine/Control/FirstPersonControl.h"
#include "Engine/Control/ThirdPersonControl.h"

#include "NanoVG/nanovg.h"

#include "Engine/Render/Mesh/Mesh.h"
#include "Engine/Render/Mesh/Shape.h"

#include "openvr.h"

#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"
#include "Engine/Render/FrameBuffer.h"
#include "Engine/UI/UIManager.h"
#include "Engine/Utils/Utils.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui_internal.h"
#include "ImGui/imgui_style.h"
#include "ImGui/imgui_impl_sdl_gl3.h"
#include "ImGui/imgui_tab.h"

#include "Engine/Utils/IdProvider.h"

#include "Engine/Sound/SoundSystem.h"

#define null nullptr

namespace NovaEngine {
	class Engine : public MouseListener, public KeyListener {
	public:
		Engine();
		virtual ~Engine();

		virtual void initDisplay();
		virtual void init() = 0;
		virtual void update() = 0;
		virtual void render() = 0;
		virtual void renderUI();
		virtual void renderImGui();

		virtual void onResize(int w, int h);

		virtual void onMouseMove(int x, int y, int dx, int dy) override;
		virtual void onMouseEvent(ButtonEvent e) override;
		virtual void onKeyEvent(KeyEvent e) override;

		void initVR();

		void start();
		void stop();
	private:
		std::vector<Spatial*> _spatialToDelete;
		std::vector<Mesh*> _meshToDelete;

		void preInit();
		void preUpdate();
		void updateVREvents();

		void renderVR();

		void preRenderImGui();
		void prepareDeleteSpatial(Spatial* sp);
		void deleteSpatials();

		glm::mat4 getHMDMatrixProjectionEye(vr::Hmd_Eye nEye);
		glm::mat4 getHMDMatrixPoseEye(vr::Hmd_Eye nEye);
		Camera* getEyeCamera(vr::Hmd_Eye nEye);

		FrameBuffer* createFrameBufferVR(int w, int h);

	protected:
		InputManager* _inputManager;
		Display* _display;
		Timer _timer;
		AssetManager* _assetManager;
		Renderer* _renderer;
		RenderManager* _renderManager;
		SpriteBatch* _spriteBatch;

		ViewPort* _viewPort;
		Camera* _camera;
		Camera* _guiCamera;

		Spatial* _mainScene;

		FirstPersonControl* _firstPersonControl;
		ThirdPersonControl* _thirdPersonControl;

		SoundSystem* _soundSystem = nullptr;

		UINode* _uiNode;
		UIManager* _uiManager;
		UIRenderer* _uiRenderer;
	};
}

#endif // !NOVA_ENGINE_H