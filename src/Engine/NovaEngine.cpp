#include "Engine/NovaEngine.h"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "Engine/Render/FrameBuffer.h"
#include "Engine/Scripting/Java/JavaScripting.h"

namespace NovaEngine {

	Engine::Engine() : _timer() {

	}

	void Engine::prepareDeleteSpatial(Spatial* sp) {
		if (std::find(_spatialToDelete.begin(), _spatialToDelete.end(), sp) == _spatialToDelete.end()) {
			_spatialToDelete.push_back(sp);
		}
		if (sp->getType() == SpatialType::GameObject) {
			GameObject* obj = static_cast<GameObject*>(sp);
			Mesh* mesh = obj->getMesh();
			if (std::find(_meshToDelete.begin(), _meshToDelete.end(), mesh) == _meshToDelete.end()) {
				_meshToDelete.push_back(mesh);
			}
		}
		for (Spatial* child : sp->getChilds()) {
			prepareDeleteSpatial(child);
		}
	}

	void Engine::deleteSpatials() {
		prepareDeleteSpatial(_mainScene);

		for (Mesh* mesh : _meshToDelete) {
			delete mesh;
		}

		for (Spatial* spatial : _spatialToDelete) {
			delete spatial;
		}
	}

	Engine::~Engine() {
		ImGui_ImplSdlGL3_Shutdown();
		delete _inputManager;
		delete _display;
		delete _assetManager;
		//delete _camera;
		delete _guiCamera;
		delete _renderer;
		deleteSpatials();
		//delete _mainScene;
		delete _uiManager;
		delete _soundSystem;

		_camera = nullptr;
	}

	void Engine::initDisplay() {
		_display->setResizable(true);
		_display->setWindowState(DisplayState::Maximized);
	}

	void Engine::start() {
		_inputManager = new InputManager();
		_display = new Display();
		_display->setInputManager(_inputManager);
		initDisplay();
		_display->setResizeListener(std::bind(&Engine::onResize, this, std::placeholders::_1, std::placeholders::_2));
		_display->create();
		preInit();
		while (_display->isRunning()) {
			_timer.update();
			_display->updateEvents();
			preUpdate();
			_display->update();
		}
	}

	void Engine::stop() {
		_display->stop();
	}

	void Engine::onResize(int w, int h) {
		_renderManager->resize(w, h);
	}

	void Engine::onMouseMove(int x, int y, int dx, int dy) {

	}

	void Engine::onMouseEvent(ButtonEvent e) {

	}

	void Engine::onKeyEvent(KeyEvent e) {

	}

	void Engine::preInit() {
		_inputManager->addMouseListener(this);
		_inputManager->addKeyListener(this);

		_assetManager = new AssetManager();
		_renderer = new Renderer();
		_assetManager->setRenderer(_renderer);
		_renderManager = new RenderManager(_renderer);

		_camera = new Camera(_display->getWidth(), _display->getHeight());
		_camera->setFrustumPerspective(75.0f, (float)_display->getWidth() / (float)_display->getHeight(), 0.01f, 10000.0f);
		_camera->setLocation(0, 0, 0);

		_guiCamera = new Camera(_display->getWidth(), _display->getHeight());
		_guiCamera->update();

		_viewPort = _renderManager->createViewPort("MainPort", _camera, false);
		_mainScene = _viewPort->getScene();

		_mainScene->addChild(_camera);

		_renderManager->createViewPort("ViewPortGui", _guiCamera, false);

		_spriteBatch = new SpriteBatch(_guiCamera, _renderManager, _assetManager->loadMaterial("Assets/Materials/Texture"));

		_uiRenderer = new UIRenderer(_display);
		_uiNode = new UINode();
		_uiManager = new UIManager(_uiRenderer, _inputManager, _uiNode);
		_inputManager->addMouseListener(_uiManager);
		_inputManager->addKeyListener(_uiManager);

		_firstPersonControl = new FirstPersonControl(_camera);
		_firstPersonControl->_flyMode = true;
		_inputManager->addKeyListener(_firstPersonControl);
		_inputManager->addMouseListener(_firstPersonControl);

		_thirdPersonControl = new ThirdPersonControl(_camera);
		_inputManager->addKeyListener(_thirdPersonControl);
		_inputManager->addMouseListener(_thirdPersonControl);

		_soundSystem = new SoundSystem();

		ImGui_ImplSdlGL3_Init(_display->getWindow());
		_display->setEventListener([](SDL_Event& e) {
			ImGui_ImplSdlGL3_ProcessEvent(&e);
		});
		ImGui_CustomStyle();

		init();
	}

	void Engine::preUpdate() {
		_renderer->clearColor(_viewPort->getBackgroundColor());
		_renderer->clearScreen(true, false, false);

		_firstPersonControl->update(_timer.getDelta() * 0.04f);
		_thirdPersonControl->update();

		_camera->update();
		_guiCamera->update();

		_mainScene->update();
		_soundSystem->update();
		update();
		
		//glEnable(GL_FRAMEBUFFER_SRGB);
		glViewport(0, 0, _camera->getWidth(), _camera->getHeight());
		render();
		//glDisable(GL_FRAMEBUFFER_SRGB);

		_uiRenderer->begin();
		renderUI();
		preRenderImGui();
		_uiRenderer->end();
		_renderer->finish();
	}

	void Engine::renderImGui() {
		
	}

	void Engine::preRenderImGui() {
		ImGui_ImplSdlGL3_NewFrame(_display->getWindow());
		renderImGui();
		ImGui::Render();
	}

	void Engine::renderUI() {
		_uiManager->updateAndRender();
	}
}