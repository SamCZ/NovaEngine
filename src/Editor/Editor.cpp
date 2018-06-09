#include "Editor/Editor.h"
#include "Engine/Utils/TextureCompressor.h"
#include "Engine/Utils/TangentGenerator.h"
#include "Editor/MetaManager.h"

#include "alc.h"
#include "AL/alut.h"

#include "LibGizmo/IGizmo.h"

#include "Engine/Terrain/HeightStorage.h"
#include "Engine/Terrain/TerrainQuad.h"
#include <thread>
#include "Engine/Utils/FastNoise.h"

#include "glm/gtc/type_ptr.hpp"

#include "ImGui/imgui_helper.h"

#include "Engine/Utils/SystemUtils.h"

ALuint source;
PointLight* pl;
DirectionalLight* _mainDirLight;

Editor::~Editor() {
	_activeProject->save(true);
}

void Editor::init() {
	_activeProject = new Project(_mainScene, _viewPort);
	_activeProject->init("TestProject");

	_assetTree = new FileTree(_activeProject->getFile(), true);
	_selectedFileTree = _assetTree;
	_activeProject->setFileTree(_assetTree);

	_firstMaterial = _assetManager->loadMaterial("Assets/Materials/Lighting");
	_firstMaterial->setUseLight(true);
	_activeProject->setTempMaterial(_firstMaterial);

	_activeProject->load(_assetManager);

	//_camera->setLocation(0, 0, 5);
	//_firstMaterial->setVector4("Color", glm::vec4(1, 1, 1, 1));

	_gizmoControl = new GizmoControl(_viewPort, _inputManager);

	_display->setMouseGrabbed(false);
	_firstPersonControl->_enabled = false;

	_renderManager->initPBR_IBL(_assetManager);

	//_defferedRendering = new DefferedRendering(_viewPort, _renderManager, _renderer, _assetManager, _spriteBatch);

	_selectedSpatial = nullptr;
	_selectedMaterial = nullptr;
	_draggedTexture = nullptr;
	_fileWindowMode = 0;
	_isShiftDown = false;
	_inspectorMode = FileType::Object3D;
	_selectedTexture = nullptr;
	_draggedMaterial = nullptr;
	_materialWindowOpened = false;
	//_draggingTexture = nullptr;
	_skyRenderingEnabled = true;

	_folderIcon = _assetManager->loadTexture("Assets/Textures/FolderIcon.png");
	_unloadedIcon = _assetManager->loadTexture("Assets/Textures/UnloadedFile.png");
	_materialIcon = _assetManager->loadTexture("Assets/Textures/MaterialTexture.png");
	_modelIcon = _assetManager->loadTexture("Assets/Textures/ModelTexture.png");
	_sceneIcon = _assetManager->loadTexture("Assets/Textures/icon_scene.png");

	_lightIcon = _assetManager->loadTexture("Assets/Textures/light.png");
	_lightBulbIcon = _assetManager->loadTexture("Assets/Textures/lightbulb.png");
	_cameraIcon = _assetManager->loadTexture("Assets/Textures/camera.png");

	_editorCamera = new Camera(_display->getWidth(), _display->getHeight());
	_editorCamera->setFrustumPerspective(75.0f, (float)_display->getWidth() / (float)_display->getHeight(), 0.01f, 10000.0f);

	for (File f : File("Assets/Materials/").listFiles()) {
		_shaders.push_back(f);
	}

	for (int i = 0; i < _shaders.size(); i++) {
		_shaderCombo += _shaders[i].getName() + '\0';
		_shadersArrayMap[i] = new File(_shaders[i]);
	}

	_display->setFileDropListener([this](std::string file, int x, int y) {
		_activeProject->loadAsset(File(file), _selectedFileTree, false);
	});

	{// SKY
		Material* skyMaterial = _assetManager->loadMaterial("Assets/Materials/Clouds");
		skyMaterial->setTexture("iChannel1", _renderManager->getEnviromentMap());
		skyMaterial->setBool("UseCubeMap", false);
		_cloudBatch = new SpriteBatch(_guiCamera, _renderManager, skyMaterial);
		_cloudBatch->getMaterial()->setVector2("view_port", glm::vec2(_editorCamera->getWidth(), _editorCamera->getHeight()));
		_cloudBatch->getMaterial()->setMatrix4("inv_proj", glm::inverse(_editorCamera->getProjectionMatrix()));
		_time = 0;
	}
	DirectionalLight* dirLight = null;
	for (Light* l : _mainScene->getLights()) {
		if (l->getType() == LightType::Directional) {
			dirLight = (DirectionalLight*)l;
			break;
		}
	}

	_post = new PostFilters(_assetManager, _viewPort, _guiCamera, _renderManager);
	//_assetTree->addData("Internal:SSAO", FileType::Material, _post->_material);

	/*
	auto erroLambd = [](GLenum source,
		GLenum type,
		GLuint id,
		GLenum severity,
		GLsizei length,
		const GLchar *message,
		const void *userParam) {
		if (type == GL_DEBUG_TYPE_ERROR || type == GL_DEBUG_TYPE_ERROR_ARB) {
			std::cout << "OpenGL err: " << message << std::endl;
		}
	};
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(erroLambd, NULL);*/



	setCameraPlayMode(false);
}

void deleteObject(Spatial* spatial) {
	if (spatial) {
		Spatial* parent = spatial->getParent();
		if (parent) {
			if (Light* light = dynamic_cast<Light*>(spatial)) {
				parent->removeLight(light);
			} else {
				parent->removeChild(spatial);
			}
			delete spatial;
		}
	}
}

void Editor::update() {
	//pl->setPosition(_camera->getLocation());
	//_mainScene->getChilds()[0]->getRotation().y += 0.1;
	
	/*for (Spatial* sp : _mainScene->getChilds()) {
		if (TerrainQuadTree* qt = dynamic_cast<TerrainQuadTree*>(sp)) {
			if (!_firstPersonControl->_flyMode) {
				_camera->getLocation().y = qt->getHeight(_camera->getLocation().x, _camera->getLocation().z) + 0.5f;
				_camera->update();
			}
		}
	}*/

	_editorCamera->update();

	_firstPersonControl->_flyMode = !_playMode;
	if (_playMode)_firstPersonControl->_enabled = true;

	for (FileTreeQueueItem& item : _assetDeleteQueue) {
		if (item.useData) {
			item.baseTree->removeData(*item.data.path);
			_assetManager->getLoadedTextures().erase((*item.data.path).getPath());
			item.data.path->deleteFile();
		} else {

		}
	}
	_assetDeleteQueue.clear();

	//renderWater();
}

void renderBoundingBox(BoundingBox* bb) {
	glColor3f(1, 0, 0);
	glBegin(GL_LINES);

	glVertex3f(bb->getMinX(), bb->getMinY(), bb->getMinZ());
	glVertex3f(bb->getMinX(), bb->getMaxY(), bb->getMinZ());

	glVertex3f(bb->getMaxX(), bb->getMinY(), bb->getMinZ());
	glVertex3f(bb->getMaxX(), bb->getMaxY(), bb->getMinZ());

	glVertex3f(bb->getMinX(), bb->getMinY(), bb->getMaxZ());
	glVertex3f(bb->getMinX(), bb->getMaxY(), bb->getMaxZ());

	glVertex3f(bb->getMaxX(), bb->getMinY(), bb->getMaxZ());
	glVertex3f(bb->getMaxX(), bb->getMaxY(), bb->getMaxZ());
	///////////////////////////////////////////////////////
	glVertex3f(bb->getMinX(), bb->getMinY(), bb->getMinZ());
	glVertex3f(bb->getMaxX(), bb->getMinY(), bb->getMinZ());

	glVertex3f(bb->getMinX(), bb->getMaxY(), bb->getMinZ());
	glVertex3f(bb->getMaxX(), bb->getMaxY(), bb->getMinZ());
	///////////////////////////////////////////////////////
	glVertex3f(bb->getMaxX(), bb->getMinY(), bb->getMinZ());
	glVertex3f(bb->getMaxX(), bb->getMinY(), bb->getMaxZ());

	glVertex3f(bb->getMaxX(), bb->getMaxY(), bb->getMinZ());
	glVertex3f(bb->getMaxX(), bb->getMaxY(), bb->getMaxZ());
	////////////////////////////////////////////////////////
	glVertex3f(bb->getMaxX(), bb->getMinY(), bb->getMaxZ());
	glVertex3f(bb->getMinX(), bb->getMinY(), bb->getMaxZ());

	glVertex3f(bb->getMaxX(), bb->getMaxY(), bb->getMaxZ());
	glVertex3f(bb->getMinX(), bb->getMaxY(), bb->getMaxZ());
	////////////////////////////////////////////////////////
	glVertex3f(bb->getMinX(), bb->getMinY(), bb->getMaxZ());
	glVertex3f(bb->getMinX(), bb->getMinY(), bb->getMinZ());

	glVertex3f(bb->getMinX(), bb->getMaxY(), bb->getMaxZ());
	glVertex3f(bb->getMinX(), bb->getMaxY(), bb->getMinZ());

	glEnd();
}

glm::vec3 pointFromCam(Camera* camera, const glm::vec3& pos) {
	static Spatial sp;
	sp.setLocation(pos);
	return camera->getModelMatrix() * sp.getModelMatrix() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
}

void glVer3(const glm::vec3& pos) {
	glVertex3f(pos.x, pos.y, pos.z);
}

void Editor::drawLightDebug(Spatial* spatial, IdProvider& ids) {
	for (Spatial* child : spatial->getChilds()) {
		drawLightDebug(child, ids);
	}

	if (Camera* cam = dynamic_cast<Camera*>(spatial)) {
		glm::vec3 posScreen = _editorCamera->getScreenCoordinates(cam->getLocation());
		if (posScreen.z < 1.0) {
			glEnable(GL_TEXTURE_2D);
			_renderer->setInvalidateShader();
			_spriteBatch->drawTexture(_cameraIcon, posScreen.x - 16, posScreen.y - 16, 32, 32, makeRGB(35, 120, 255), true);
			glDisable(GL_TEXTURE_2D);
			glBindVertexArray(0);
			glUseProgram(0);

			glLineWidth(0.5);
			glColor3f(0.0, 1.0, 1.0);
			glBegin(GL_LINES);
			{
				float far_plane = 0.5f;
				float near_plane = 0.05f;

				glVer3(pointFromCam(cam, { 0, 0, 0 }));
				glVer3(pointFromCam(cam, { cam->getFrustumLeft() - near_plane, cam->getFrustumBottom() - near_plane, -cam->getFrustumNear() - near_plane }));

				glVer3(pointFromCam(cam, { 0, 0, 0 }));
				glVer3(pointFromCam(cam, { cam->getFrustumRight() + near_plane, cam->getFrustumBottom() - near_plane, -cam->getFrustumNear() - near_plane }));

				glVer3(pointFromCam(cam, { 0, 0, 0 }));
				glVer3(pointFromCam(cam, { cam->getFrustumLeft() - near_plane, cam->getFrustumTop() + near_plane, -cam->getFrustumNear() - near_plane }));

				glVer3(pointFromCam(cam, { 0, 0, 0 }));
				glVer3(pointFromCam(cam, { cam->getFrustumRight() + near_plane, cam->getFrustumTop() + near_plane, -cam->getFrustumNear() - near_plane }));



				glVer3(pointFromCam(cam, { cam->getFrustumLeft() - near_plane, cam->getFrustumBottom() - near_plane, -cam->getFrustumNear() - near_plane }));
				glVer3(pointFromCam(cam, { cam->getFrustumRight() + near_plane, cam->getFrustumBottom() - near_plane, -cam->getFrustumNear() - near_plane }));

				glVer3(pointFromCam(cam, { cam->getFrustumLeft() - near_plane, cam->getFrustumTop() + near_plane, -cam->getFrustumNear() - near_plane }));
				glVer3(pointFromCam(cam, { cam->getFrustumRight() + near_plane, cam->getFrustumTop() + near_plane, -cam->getFrustumNear() - near_plane }));

				glVer3(pointFromCam(cam, { cam->getFrustumLeft() - near_plane, cam->getFrustumTop() + near_plane, -cam->getFrustumNear() - near_plane }));
				glVer3(pointFromCam(cam, { cam->getFrustumLeft() - near_plane, cam->getFrustumBottom() - near_plane, -cam->getFrustumNear() - near_plane }));

				glVer3(pointFromCam(cam, { cam->getFrustumRight() + near_plane, cam->getFrustumTop() + near_plane, -cam->getFrustumNear() - near_plane }));
				glVer3(pointFromCam(cam, { cam->getFrustumRight() + near_plane, cam->getFrustumBottom() - near_plane, -cam->getFrustumNear() - near_plane }));



				glVer3(pointFromCam(cam, { cam->getFrustumLeft() - near_plane, cam->getFrustumBottom() - near_plane, -cam->getFrustumNear() - near_plane }));
				glVer3(pointFromCam(cam, { cam->getFrustumLeft() - far_plane, cam->getFrustumBottom() - far_plane, -far_plane + 0.1f }));

				glVer3(pointFromCam(cam, { cam->getFrustumRight() + near_plane, cam->getFrustumBottom() - near_plane, -cam->getFrustumNear() - near_plane }));
				glVer3(pointFromCam(cam, { cam->getFrustumRight() + far_plane, cam->getFrustumBottom() - far_plane, -far_plane + 0.1f }));

				glVer3(pointFromCam(cam, { cam->getFrustumLeft() - near_plane, cam->getFrustumTop() + near_plane, -cam->getFrustumNear() - near_plane }));
				glVer3(pointFromCam(cam, { cam->getFrustumLeft() - far_plane, cam->getFrustumTop() + far_plane, -far_plane + 0.1f }));

				glVer3(pointFromCam(cam, { cam->getFrustumRight() + near_plane, cam->getFrustumTop() + near_plane, -cam->getFrustumNear() - near_plane }));
				glVer3(pointFromCam(cam, { cam->getFrustumRight() + far_plane, cam->getFrustumTop() + far_plane, -far_plane + 0.1f }));



				glVer3(pointFromCam(cam, { cam->getFrustumLeft() - far_plane, cam->getFrustumBottom() - far_plane, -far_plane + 0.1f }));
				glVer3(pointFromCam(cam, { cam->getFrustumRight() + far_plane, cam->getFrustumBottom() - far_plane, -far_plane + 0.1f }));

				glVer3(pointFromCam(cam, { cam->getFrustumLeft() - far_plane, cam->getFrustumTop() + far_plane, -far_plane + 0.1f }));
				glVer3(pointFromCam(cam, { cam->getFrustumRight() + far_plane, cam->getFrustumTop() + far_plane, -far_plane + 0.1f }));

				glVer3(pointFromCam(cam, { cam->getFrustumLeft() - far_plane, cam->getFrustumTop() + far_plane, -far_plane + 0.1f }));
				glVer3(pointFromCam(cam, { cam->getFrustumLeft() - far_plane, cam->getFrustumBottom() - far_plane, -far_plane + 0.1f }));

				glVer3(pointFromCam(cam, { cam->getFrustumRight() + far_plane, cam->getFrustumTop() + far_plane, -far_plane + 0.1f }));
				glVer3(pointFromCam(cam, { cam->getFrustumRight() + far_plane, cam->getFrustumBottom() - far_plane, -far_plane + 0.1f }));
			}
			glEnd();
			glLineWidth(1.0);
		}
	}

	for (Light* light : spatial->getLights()) {
		glm::vec3& pos = light->getLocation();

		glm::vec3 posScreen = _editorCamera->getScreenCoordinates(pos);
		if (posScreen.z < 1.0) {
			Texture* icon = nullptr;
			if (light->getType() == LightType::Ambient || light->getType() == LightType::Directional) {
				icon = _lightIcon;
			} else if (light->getType() == LightType::Point) {
				icon = _lightBulbIcon;
			}
			if (icon != nullptr) {
				glEnable(GL_TEXTURE_2D);
				_renderer->setInvalidateShader();
				_spriteBatch->drawTexture(icon, posScreen.x - 16, posScreen.y - 16, 32, 32, light->getColor(), true);
				glDisable(GL_TEXTURE_2D);
				glBindVertexArray(0);
				glUseProgram(0);
			}
		}

		if (DirectionalLight* dl = dynamic_cast<DirectionalLight*>(light)) {
			glm::vec3& dir = dl->getDirection();

			glColor3f(0.5, 0.5, 1.0);
			glBegin(GL_LINES);
			glVertex3f(pos.x, pos.y, pos.z);
			glVertex3f(pos.x + dir.x * 1.5, pos.y + dir.y * 1.5, pos.z + dir.z * 1.5);

			glColor3f(dl->getColor().r, dl->getColor().g, dl->getColor().b);
			float stepSize = 360.0f / 10.0f;
			float nowAngle = 0.0f;
			for (int steps = 0; steps < 10; steps++) {
				glm::vec3 startPos = pos;
				glm::vec3 endPos = glm::vec3(pos.x + dir.x * 1, pos.y + dir.y * 1, pos.z + dir.z * 1);

				float ax = glm::cos(glm::radians(nowAngle)) * 0.1f;
				float ay = glm::sin(glm::radians(nowAngle)) * 0.1f;

				startPos.x += ax;
				startPos.y += ay;
				//startPos.z += ax - ay;

				endPos.x += ax;
				endPos.y += ay;
				//endPos.z += ax - ay;

				glVertex3f(startPos.x, startPos.y, startPos.z);
				glVertex3f(endPos.x, endPos.y, endPos.z);

				nowAngle += stepSize;
			}
			glEnd();
		}
	}
}

void Editor::render() {
	_renderer->clearScreen(true, true, false);
	_renderer->clearColor(makeRGB(0, 0, 0));
	DirectionalLight* light = nullptr;
	for (Light* l : _mainScene->getLights()) {
		if (l->getType() == LightType::Directional) {
			light = (DirectionalLight*)l;
			break;
		}
	}
	if (_skyRenderingEnabled) {
		if (light != nullptr) {
			glm::vec3 dir = light->getDirection();
			if (dir == glm::vec3(0.0f)) {
				_cloudBatch->getMaterial()->setVector3("LightDir", glm::normalize(glm::vec3(0, -1, 0)));
			} else {
				_cloudBatch->getMaterial()->setVector3("LightDir", glm::normalize(dir));
			}
		}
		_cloudBatch->getMaterial()->setFloat("iGlobalTime", _time);
		_cloudBatch->getMaterial()->setMatrix4("inv_view", glm::inverse(getActiveCamera()->getViewMatrix()));
		_cloudBatch->drawTexture(nullptr, 0, 0, getActiveCamera()->getWidth(), getActiveCamera()->getHeight());
	}

	for (Light* l : _mainScene->getLights()) {
		if (l->getType() == LightType::Point) {
			PointLight* pl = (PointLight*)l;
			if (!pl->hasDepthMap() || _updateShadowMaps) {
				pl->setDepthMap(_renderer->renderViewPortScene(_assetManager, _viewPort, pl->getPosition(), _renderManager));
				//_cloudBatch->getMaterial()->setTexture("iChannel1", pl->getDepthMap());
				//_cloudBatch->getMaterial()->setBool("UseCubeMap", true);
			}
		}
	}

	//_renderManager->renderViewPort(_viewPort);
	_post->render();
	//while (true);

	for (FileData& mmat : _activeProject->getCreatedMaterials()) {
		((Material*)mmat.data)->updatePreview(_renderManager);
	}

	_time += (0.0001f * _timer.getDelta()) * _timeMult;

	if (_playMode) {
		glEnable(GL_TEXTURE_2D);
		_renderer->setInvalidateShader();
		return;
	}

	glBindVertexArray(0);
	glUseProgram(0);
	glDisable(GL_TEXTURE_2D);
	glLoadIdentity();
	glLoadMatrixf(&_editorCamera->getProjectionViewMatrix()[0][0]);
	glLineWidth(1.0f);

	//GIZMO
	_gizmoControl->draw();

	IdProvider dlIp;
	drawLightDebug(_mainScene, dlIp);

	glEnable(GL_TEXTURE_2D);
	_renderer->setInvalidateShader();
}

void Editor::renderUI() {
	//_defferedRendering->showRendering();
	_post->getResult(_spriteBatch);
	//_spriteBatch->drawTexture(_post->getResult(), 0, 0, _camera->getWidth(), _camera->getHeight());
	Engine::renderUI();
	//_spriteBatch->drawTexture(_ssaoFilter->getResult(), 0, 0, _camera->getWidth(), _camera->getHeight());

	//_spriteBatch->drawTexture(_terrainTexture, 0, 0);
	//_uiRenderer->drawImage(_terrainTexture, 0, 0, 1024, 1024, 0);

	/*if (_paint) {
		_terrainTexture->doPaintRadius(_inputManager->getMouseX(), _inputManager->getMouseY(), 100, 0, 0.1);
	}*/
	//postFiltersAll->showScreen();

	static float refresh_time = 0.0f;
	if (refresh_time == 0.0f)
		refresh_time = ImGui::GetTime(); 
	while (refresh_time < ImGui::GetTime()) {
		_display->setTitleAdditional(" | FPS: " + std::to_string((int)_timer.getFrameRate()));
		refresh_time += 1.0f / 60.0f;
	}

	/*for (Spatial* sp : _mainScene->getChilds()) {
		if (TerrainQuadTree* qt = dynamic_cast<TerrainQuadTree*>(sp)) {
			if (!_firstPersonControl->_flyMode) {
				float yh = qt->getHeight(_camera->getLocation().x, _camera->getLocation().z);
				std::cout << yh << std::endl;
			}
		}
	}*/

	//_uiRenderer->drawText("FPS: " + std::to_string((int)_timer.getFrameRate()), 10, 30, 16, makeRGB(120, 120, 120));

	/*float h = _quadTreeTest->getHeight(_camera->getLocation().x, _camera->getLocation().z);
	_mainScene->find("TEST")->getLocation() = _camera->getLocation();
	_mainScene->find("TEST")->getLocation().y = h + 0.5f;
	_uiRenderer->drawText("Y: " + std::to_string(h), 10, 45, 16, makeRGB(20, 100, 200));*/
}

void Editor::drawSceneInspector(Spatial* spatial, Spatial*& selected, Material*& selectedMaterial, IdProvider& provider, GizmoControl* gControl, FileType& inspectorMode) {
	static int selection_mask = (1 << 2);
	int level = provider.getId();
	ImGuiTreeNodeFlags node_flags = (level == 0 ? ImGuiTreeNodeFlags_DefaultOpen : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | (spatial == selected ? ImGuiTreeNodeFlags_Selected : 0);
	bool node_open = false;

	std::string name = spatial->getName();
	if (_draggingSpatial == spatial) {
		name += " <-";
	}

	//if(spatial == draggingSpatial) return;
	if (spatial->getChildCount() > 0) {
		node_open = ImGui::TreeNodeEx((void*)(intptr_t)level, node_flags, "%s", LatinToAscii(name).c_str());
	} else {
		bool sel = spatial == selected;
		ImGui::TreeNodeEx((void*)(intptr_t)level, node_flags | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Leaf, "%s", LatinToAscii(name).c_str());
	}
	if (ImGui::IsMouseDragging() && ImGui::IsItemActive() && _draggingSpatial == nullptr) {
		_draggingSpatial = spatial;
	}
	if (!ImGui::IsMouseDown(0)) {
		if (_draggingSpatial != nullptr && _draggingSpatial != spatial && ImGui::IsItemHovered()) {
			/*std::cout << spatial->getName() << std::endl;
			std::cout << draggingSpatial->getName() << std::endl << std::endl;*/
			_draggingSpatial->getParent()->removeChild(_draggingSpatial);
			spatial->addChild(_draggingSpatial);
			_draggingSpatial = nullptr;
		}
	}
	if (ImGui::BeginPopupContextItem(std::to_string(level + 1000).c_str())) {
		if (ImGui::Button(("Duplicate##" + std::to_string(level + 2000)).c_str())) {
			Spatial* cloned = spatial->clone();
			if (spatial->getParent() != nullptr) {
				spatial->getParent()->addChild(cloned);

				selected = cloned;
				gControl->setControl(cloned);
				inspectorMode = FileType::Object3D;
			}
			ImGui::CloseCurrentPopup();
		}
		if (ImGui::Button(("Delete##" + std::to_string(level + 2000)).c_str())) {
			deleteObject(spatial);
			selected = nullptr;
			gControl->setControl(nullptr);
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	if (ImGui::IsItemClicked()) {
		selected = spatial;
		if (spatial != _mainScene) {
			gControl->setControl(selected);
			inspectorMode = FileType::Object3D;
		} else {
			gControl->setControl(nullptr);
			inspectorMode = FileType::Object3D;
		}
	}
	if (node_open) {
		for (Spatial* child : spatial->getChilds()) {
			drawSceneInspector(child, selected, selectedMaterial, provider, gControl, inspectorMode);
		}
		for (Spatial* child : spatial->getLights()) {
			drawSceneInspector(child, selected, selectedMaterial, provider, gControl, inspectorMode);
		}
		ImGui::TreePop();
	}
}

bool Editor::drawParamData(MaterialParam* param, TechniqueDef* technique) {
	if (!param) return false;
	bool update = false;
	ImGui::Text(param->Label.c_str());
	//ImGui::Separator();
	if (param->Type == VarType::Texture) {
		Texture* texture = nullptr;
		if (technique != nullptr) {
			texture = technique->getTextureFromParam(param);
		}
		if (texture == null) {
			ImGui::Text("Texture not set");
			ImGui::Image((ImTextureID)NULL, ImVec2(128, 128));
		} else {
			ImVec2 tSize = getScaledTexByHeight(texture, 128);
			ImGui::Image((ImTextureID)texture->getTextureId(), tSize);
		}
		Texture* droppedTexture = nullptr;
		if (IsDroppedData("Assets:Texture", (void*&)droppedTexture)) {
			technique->setParam<Texture*>(param->Name, droppedTexture);
			param->NeedsUpdate = true;
			update = true;
		}
		if (texture != null) {
			ImGui::SameLine();
			ImGui::Text(texture->getName().c_str());
			ImGui::SameLine();
			ImGui::Text((std::to_string(texture->getWidth()) + "x" + std::to_string(texture->getHeight())).c_str());
		}
		MaterialVarTexture& tdata = param->getData<MaterialVarTexture>();
		DrawText("Unit: " + std::to_string(tdata.ActiveUnit));
	} else {
		std::string id = "##Input" + std::to_string(GetNextId());
		if (param->Type == VarType::Float) {
			update |= DragFloat(id, param->getDataPointer<float>(), 0.1f);
		} else if (param->Type == VarType::Int) {
			update |= DragInt(id, param->getDataPointer<int>());
		} else if (param->Type == VarType::Vector2) {
			update |= drawVectorInput2(id, param->getData<glm::vec2>());
		} else if (param->Type == VarType::Vector3) {
			update |= drawVectorInput(id, param->getData<glm::vec3>());
			update |= drawColorPicker3(id, (float*)param->getDataPointer<glm::vec3>());
		} else if (param->Type == VarType::Vector4) {
			update |= drawVectorInput4(id, param->getData<glm::vec4>());
			update |= drawColorPicker(id, (float*)param->getDataPointer<glm::vec4>());
		} else if (param->Type == VarType::Bool) {
			update |= Checkbox("", param->getDataPointer<bool>());
		}
	}
	if (update) {
		param->NeedsUpdate = true;
		return true;
	}
}

void Editor::drawInspector(Spatial* obj, Material*& _selectedMaterial, Material* draggedMaterial, FileTreeQueueItem* item, const FileType& mode, bool &_materialWindowOpened, Camera* camera) {
	if (ImGui::Selectable("Object", _inspectorSceneMode == false, NULL, ImVec2(ImGui::GetWindowSize().x / 2.0f, 0))) {
		_inspectorSceneMode = false;
	}
	ImGui::SameLine();
	if (ImGui::Selectable("Global", _inspectorSceneMode == true, NULL, ImVec2(ImGui::GetWindowSize().x / 2.0f, 0))) {
		_inspectorSceneMode = true;
	}
	ImGui::Separator();

	if (!_inspectorSceneMode) {
		if (mode == FileType::Texture && item != nullptr) {
			Texture* texture = static_cast<Texture*>(item->data.data);
			GLuint id = texture->getTextureId();
			ImGui::Image((ImTextureID)id, { 230, 230 });

			ImGui::Text(texture->getName().c_str());
			ImGui::Separator();

			ImGui::Text("Normal map");
			ImGui::Checkbox("##TextureInspectorNormalMap", &texture->isNormalMap());
			ImGui::Separator();

			ImGui::Text("Anisotropic Filter");
			ImGui::SliderInt("##TextureInspectorAnisotropicFilter", &texture->getAnisotropicFilter(), 0, 12);
			ImGui::Separator();

			ImGui::Text("Filter mode");
			static int filter_mode = texture->minificationFilter == MinFilter::Nearest ? 0 : (texture->minificationFilter == MinFilter::Bilinear ? 1 : (texture->minificationFilter == MinFilter::Trilinear ? 2 : 0));
			if (ImGui::Combo("##FilterMode", &filter_mode, "Point (no filter)\0Bilinear\0Trilinear\0")) {
				switch (filter_mode) {
					case 0:
					texture->minificationFilter = MinFilter::Nearest;
					texture->magnificationFilter = MagFilter::Nearest;
					break;
					case 1:
					texture->minificationFilter = MinFilter::Bilinear;
					texture->magnificationFilter = MagFilter::Bilinear;
					break;
					case 2:
					texture->minificationFilter = MinFilter::Trilinear;
					texture->magnificationFilter = MagFilter::Bilinear;
					break;
				}
			}
			ImGui::Separator();

			ImGui::Text("Wrap mode");
			static int wrap_mode = texture->getWrap(WrapAxis::S) == WrapMode::Repeat ? 0 : 1;
			if (ImGui::Combo("##WrapMode", &wrap_mode, "Repeat\0Clamp")) {
				if (wrap_mode == 0) {
					texture->setWrap(WrapMode::Repeat);
				} else if (wrap_mode == 1) {
					texture->setWrap(WrapMode::ClampToEdge);
				}
			}
			ImGui::Separator();

			if (ImGui::Button("Save##TextureIspectorSave")) {
				texture->setNeedsUpdate(true);
				MetaManager::saveTextureMeta(texture, *item->data.path);
			}

			return;
		}
		if (obj == nullptr) return;
		GameObject* objInst = nullptr;
		if (objInst = dynamic_cast<GameObject*>(obj)) {
			ImGui::Text("Type: GameObject");
		} else {
			ImGui::Text("Type: Scene");
		}
		Light* light = nullptr;
		bool isLight = (light = dynamic_cast<Light*>(obj));

		static char name[64];
		memset(name, 0x00, 64);
		obj->getName().copy(name, 64);
		if (ImGui::InputText("Name", name, 64)) {
			obj->setName(std::string(name));
		}

		ImGui::Separator();
		ImGui::Checkbox("Enabled##SelectedSpatialEnableda", &obj->isVisible());
		ImGui::Separator();

		if (!isLight && obj != _mainScene) {
			if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
				ImGui::Text("Location");
				ImGui::DragFloat("X##Loc", &obj->getLocation().x, 0.1f);
				ImGui::DragFloat("Y##Loc", &obj->getLocation().y, 0.1f);
				ImGui::DragFloat("Z##loc", &obj->getLocation().z, 0.1f);

				ImGui::Text("Rotation");
				{
					ImGui::SameLine();
					if (ImGui::Button("Reset##RotationReset")) {
						obj->setRotation(0, 0, 0);
					}
				}
				ImGui::DragFloat("X##Rot", &obj->getRotation().x, 0.1f);
				ImGui::DragFloat("Y##Rot", &obj->getRotation().y, 0.1f);
				ImGui::DragFloat("Z##Rot", &obj->getRotation().z, 0.1f);

				ImGui::Text("Scale");
				{
					ImGui::SameLine();
					if (ImGui::Button("Reset##ScaleReset")) {
						obj->setScale(1, 1, 1);
					}
				}
				ImGui::DragFloat("X##Scale", &obj->getScale().x, 0.1f);
				ImGui::DragFloat("Y##Scale", &obj->getScale().y, 0.1f);
				ImGui::DragFloat("Z##Scale", &obj->getScale().z, 0.1f);
			}
		}

		if (!isLight) {
			if (ImGui::CollapsingHeader("Scripts", ImGuiTreeNodeFlags_DefaultOpen)) {
				static int script_index = 0;
				ImGui::Combo("##AddScriptList", &script_index, ComboData({ "Particle emitter" }));
				ImGui::SameLine();
				if (ImGui::Button("Add##SpatialAddScript")) {
					if (script_index == 0) {
						obj->addComponent(new ParticleEmitter());
					}
				}
				ImGui::Separator();
				for (Component* cpn : obj->getComponents()) {
					DrawText(cpn->getName(), makeRGB(255, 255, 255));
					for (auto& iv : cpn->getVariables()) {
						if (!iv.second.isButton() && iv.second.getParam()) {
							drawParamData(iv.second.getParam(), nullptr);
						} else {
							ImGui::Button(iv.second.getName().c_str());
						}
					}
					if (OnButton("Remove component")) {
						obj->removeComponent(cpn);
						delete cpn;
					}
					ImGui::Separator();
				}
			}
		}

		if (isLight) {
			std::string lightName = "Unknown";
			AmbientLight* ambientLight = nullptr;
			DirectionalLight* dirLight = nullptr;
			PointLight* pointLight = nullptr;
			if (ambientLight = dynamic_cast<AmbientLight*>(light)) {
				lightName = "Ambient light";
			} else if (dirLight = dynamic_cast<DirectionalLight*>(light)) {
				lightName = "Directional light";
			} else if (pointLight = dynamic_cast<PointLight*>(light)) {
				lightName = "Point light";
			}

			ImGui::Text("Color:");
			drawColorPicker("##LightColor", light->getColor());
			DragFloat("Intensity", &light->getIntensity(), 0.025f);
			ImGui::SliderFloat("Intensity", &light->getIntensity(), 0.0f, 1.0f, "%.3f");
			if (dirLight != nullptr) {
				ImGui::Text("Position:");
				drawVectorInput("DirLightPosition", dirLight->getPosition(), 0.1);
				ImGui::Text("Rotation:");
				if (drawVectorInput("DirLightRotation", dirLight->getRotation(), 0.1)) {
					dirLight->updateRotation();
				}
				ImGui::Text("Direction:");
				if (drawVectorInput("DirLightVector", dirLight->getDirection(), 0.1)) {
					//dirLight->setDirection(glm::normalize(dirLight->getDirection()));
				}
			}
			if (pointLight != nullptr) {
				ImGui::Text("Position:");
				drawVectorInput("PointLigtPos", pointLight->getPosition());
				ImGui::Text("Radius:");
				float rad = pointLight->getRadius();
				if (ImGui::DragFloat("##PointLightRad", &rad, 0.05f)) {
					pointLight->setRadius(rad);
				}
			}
			if (ImGui::Button("Delete##DeleteLight")) {
				light->getParent()->removeLight(light);
			}

			return;
		}

		if (ImGui::CollapsingHeader("Queue bucket")) {
			static int bucket_mode = 0;
			if (obj->getRenderOrder() == RenderOrder::Opaque) {
				bucket_mode = 0;
			} else if (obj->getRenderOrder() == RenderOrder::Trasparent) {
				bucket_mode = 1;
			} else if (obj->getRenderOrder() == RenderOrder::Sky) {
				bucket_mode = 2;
			} else if (obj->getRenderOrder() == RenderOrder::Gui) {
				bucket_mode = 3;
			}
			if (ImGui::Combo("##RenderQueueMode", &bucket_mode, "Opaque\0Trasparent\0Sky\0Gui\0")) {
				switch (bucket_mode) {
					case 0:
					obj->setRenderOrder(RenderOrder::Opaque);
					break;
					case 1:
					obj->setRenderOrder(RenderOrder::Trasparent);
					break;
					case 2:
					obj->setRenderOrder(RenderOrder::Sky);
					break;
					case 3:
					obj->setRenderOrder(RenderOrder::Gui);
					break;
				}
			}
		}
		/*
		if (ImGui::CollapsingHeader("Lights")) {
			static int inputs_mode = 0;
			ImGui::Combo("##LightList", &inputs_mode, "Ambient Light\0Directional Light\0Point Light\0Spot Light\0");
			ImGui::SameLine();
			if (ImGui::Button("Add light##SpatialAddLight")) {
				switch (inputs_mode) {
					case 0:
					obj->addLight(new AmbientLight());
					break;
					case 1:
					obj->addLight(new DirectionalLight());
					break;
					case 2:
					obj->addLight(new PointLight(camera->getLocation(), makeRGB(255, 255, 255), 1));
					break;
					case 3:
					//obj->addLight(new SpotLight);
					break;
				}
			}
			ImGui::Separator();

			int lc = 0;
			for (Light* light : obj->getLights()) {
				std::string lightName = "Unknown";
				AmbientLight* ambientLight = nullptr;
				DirectionalLight* dirLight = nullptr;
				PointLight* pointLight = nullptr;
				if (ambientLight = dynamic_cast<AmbientLight*>(light)) {
					lightName = "Ambient light";
				} else if (dirLight = dynamic_cast<DirectionalLight*>(light)) {
					lightName = "Directional light";
				} else if (pointLight = dynamic_cast<PointLight*>(light)) {
					lightName = "Point light";
				}
				if (ImGui::TreeNodeEx((void*)(intptr_t)lc++, 0, lightName.c_str())) {
					ImGui::Text("Color:");
					drawColorPicker("##LightColor", light->getColor());
					if (dirLight != nullptr) {
						ImGui::Text("Position:");
						drawVectorInput("DirLightPosition", dirLight->getPosition(), 0.1);
						ImGui::Text("Rotation:");
						if (drawVectorInput("DirLightRotation", dirLight->getRotation(), 0.1)) {
							dirLight->updateRotation();
						}
						ImGui::Text("Direction:");
						if (drawVectorInput("DirLightVector", dirLight->getDirection(), 0.1)) {
							//dirLight->setDirection(glm::normalize(dirLight->getDirection()));
						}
					}
					if (pointLight != nullptr) {
						ImGui::Text("Position:");
						drawVectorInput("PointLigtPos", pointLight->getPosition());
						ImGui::Text("Radius:");
						float rad = pointLight->getRadius();
						if (ImGui::DragFloat("##PointLightRad", &rad, 0.5f)) {
							pointLight->setRadius(rad);
						}
					}
					if (ImGui::Button("Delete##DeleteLight")) {
						obj->removeLight(light);
					}
					ImGui::TreePop();
				}
			}
		}
		*/

		TerrainQuadTree* terrainObj = nullptr;
		if (objInst != nullptr || (terrainObj = dynamic_cast<TerrainQuadTree*>(obj))) {
			if (ImGui::CollapsingHeader("Material##ObjectMaterialEditor", ImGuiTreeNodeFlags_DefaultOpen)) {
				Material* material = nullptr;
				if (objInst != nullptr) {
					material = objInst->getMaterial();
				} else {
					material = terrainObj->getMaterial();
				}
				if (true) {
					static bool canDrop = false;
					if (canDrop) {
						ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, ImVec4(0.2, 0.5, 0.8, 1.0));
					}
					BeginFrame(0, 25, false, 5);
					if (canDrop) {
						ImGui::PopStyleColor();
					}
					ImGui::Text("Material: %s", material == nullptr ? "None" : material->getFilename().c_str());
					EndFrame();

					canDrop = CanDropData("Assets:Material");
					Material* dropMat = nullptr;
					if (IsDroppedData("Assets:Material", (void*&)dropMat)) {
						if (objInst != nullptr) {
							objInst->setMaterial(dropMat);
						} else {
							terrainObj->setMaterial(dropMat);
						}
					}
					if (ImGui::BeginPopupContextItem("Object material menu##ObjectMaterialMenu")) {
						if (ImGui::Button("Paste material")) {
							if (objInst != nullptr) {
								objInst->setMaterial(draggedMaterial);
							} else {
								terrainObj->setMaterial(draggedMaterial);
							}
							ImGui::CloseCurrentPopup();
						}
						if (ImGui::Button("Remove material")) {
							if (objInst != nullptr) {
								objInst->setMaterial(nullptr);
							} else {
								terrainObj->setMaterial(nullptr);
							}
							ImGui::CloseCurrentPopup();
						}
						ImGui::EndPopup();
					}
				}
				if (ImGui::IsItemClicked()) {
					Material* material = objInst->getMaterial();
					_selectedMaterial = material;
					_materialWindowOpened = true;
				}
			}
		}

		if (objInst == nullptr) return;

		if (ImGui::CollapsingHeader("Mesh##ObjectMeshDataInfo", ImGuiTreeNodeFlags_DefaultOpen)) {
			Mesh* mesh = objInst->getMesh();
			if (true) {
				ImGui::BeginChildFrame((unsigned int)456, ImVec2(0, 25));
				ImGui::Text("Mesh: %s", mesh != nullptr ? "Complex" : "None");
				ImGui::EndChildFrame();
				if (ImGui::IsItemHovered() && mesh != nullptr) {
					ImGui::BeginTooltip();
					VertexBuffer* posBuff = mesh->getVertexBuffer(MeshBuffer::Type::Position);
					VertexBuffer* indexBuff = mesh->getVertexBuffer(MeshBuffer::Type::Index);
					VertexBuffer* normalBuff = mesh->getVertexBuffer(MeshBuffer::Type::Normal);
					VertexBuffer* texBuff = mesh->getVertexBuffer(MeshBuffer::Type::TexCoord);

					ImGui::Text("Vertices: %s", posBuff != null ? std::to_string(posBuff->getBuffer()->size() / posBuff->getNumComponents()) : "None");
					ImGui::Text("Indices: %s", indexBuff != null ? std::to_string(indexBuff->getBuffer()->size()) : "None");
					ImGui::Text("Normals: %s", normalBuff != null ? std::to_string(normalBuff->getBuffer()->size() / 3) : "None");
					ImGui::Text("TexCoordinates: %s", texBuff != null ? std::to_string(texBuff->getBuffer()->size() / 2) : "None");
					
					ImGui::Text("Attrib size: %d", mesh->_enabledVertexAttribArrays);
					for (int i = 0; i < mesh->_enabledVertexAttribArrays; i++) {
						ImGui::Text(" Attrib: %d = %d", i, mesh->_vertexAttribArrays[i]);
					}
					ImGui::EndTooltip();
				}
			}
		}
	} else {
		//Global
		ImGui::DragFloat("Time multiplier", &_timeMult);
		Checkbox("Sky enabled", &_skyRenderingEnabled);
		drawColorPicker("Sky color", _viewPort->getBackgroundColor());
		if (Checkbox("Use cubemap in sky", _cloudBatch->getMaterial()->getTechnique()->getParam("UseCubeMap")->getDataPointer<bool>())) {
			_cloudBatch->getMaterial()->getTechnique()->getParam("UseCubeMap")->NeedsUpdate = true;
		}
		ImGui::DragInt("Grass distance", &_grassDistance);
		ImGui::DragFloat("Grass density", &_grassDensity);
		_updateShadowMaps = OnButton("Update shadowmaps");
	}
}

void drawFileTree(FileTree* tree, int& index, FileTree*& selectedTreeItem) {
	if(!tree->isDirectory()) return;
	ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | (tree == selectedTreeItem ? ImGuiTreeNodeFlags_Selected : 0);
	bool open = ImGui::TreeNodeEx((void*)(intptr_t)(index++), node_flags, (LatinToAscii(tree->getPath().getName())).c_str());
	if (ImGui::IsItemClicked()) {
		selectedTreeItem = tree;
	}
	if (open) {
		for (FileTree* subTree : tree->getChilds()) {
			drawFileTree(subTree, index, selectedTreeItem);
		}
		ImGui::TreePop();
	}
}

void Editor::drawMaterialWindow() {
	if(_selectedMaterial == null) return;
	Material* mat = _selectedMaterial;
	static Material* lastMaterialEditState = nullptr;

	mat->setUseLight(true);

	TechniqueDef* technique = mat->getTechnique("Default");

	// Head part
	BeginFrame(128, 128, false, 0);
	{
		if (mat->getPreviewTexture() != nullptr) {
			ImGui::Image((ImTextureID)mat->getPreviewTexture()->getTextureId(), { 128, 128 });
		} else {
			ImGui::Image((ImTextureID)_materialIcon->getTextureId(), { 128, 128 });
		}
	}
	EndFrame();

	ImGui::SameLine(0, -1);

	BeginFrame(128, false);
	{
		DrawText("Info");
		if (ImGui::Button("Recompile")) {
			mat->recompile();
		}
	}
	EndFrame();
	// Body part

	static ParamBlock* SelectedBlock = nullptr;
	static int SelectedBlockIndex = -1;
	int VaryingBlockIndex = 0;
	static ParamBlock StateBlock = { "States" };

	if (lastMaterialEditState != mat) {
		SelectedBlock = nullptr;
		SelectedBlockIndex = -1;
	}

	BeginFrame(128, 0, true, 0);
	{
		ImGui::BeginGroup();
		{
			static auto blockSelectorLambda = [&](ParamBlock& block) {
				if (SelectableButton(block.Name, SelectedBlockIndex == VaryingBlockIndex) || SelectedBlock == nullptr) {
					SelectedBlock = &block;
					SelectedBlockIndex = VaryingBlockIndex;
				}
				VaryingBlockIndex++;
			};

			for (ParamBlock& block : technique->getParamBlocks()) {
				blockSelectorLambda(block);
			}
			blockSelectorLambda(technique->getOtherParamsBlock());
			blockSelectorLambda(StateBlock);
		}
		ImGui::EndGroup();
	}
	EndFrame();

	ImGui::SameLine(0, -1);

	BeginFrame(true);
	{
		if (VaryingBlockIndex - 1 == SelectedBlockIndex) {
			RenderState& state = technique->getRenderState();
			ImGui::Combo("Blending", (int*)&state.blendMode, ComboData({"Off", "Additive", "PremultAlpha", "AlphaAdditive", "Color", "Alpha", "Screen"}));
			ImGui::Combo("Face cull", (int*)&state.faceCull, ComboData({ "Off", "Front", "Back", "FrontAndBack" }));
			Checkbox("Wireframe", &state.wireframe);
			DragFloat("Line width", &state.lineWidth, 0.05);
		} else if (SelectedBlock != nullptr) {
			for (ParamSubBlock* subBlock : SelectedBlock->SubBlocks) {
				bool enabled = true;
				bool defineExist = false;
				if (subBlock->Define.length() != 0) {
					defineExist = true;
					enabled = technique->getDefineState(subBlock->Define);
					if (Checkbox("", &technique->getDefineState(subBlock->Define))) {
						enabled = !enabled;
						technique->setDefine(subBlock->Define, enabled);
					}
					ImGui::SameLine();
				}
				
				CollapsingHeader(subBlock->Name, true, enabled ? 0 : (ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_AllowOverlapMode | ImGuiTreeNodeFlags_Leaf));
				if (enabled) {
					for (MaterialParam* param : subBlock->Params) {
						if (drawParamData(param, technique)) {
							mat->setUpdatePreview();
						}
					}
					ImGui::Separator();
				}
			}
		}
	}
	EndFrame();

	lastMaterialEditState = mat;
}

void Editor::drawFileWindow() {
	ImGui::BeginChildFrame(20, ImVec2(220, 0));
	{// Type selector
		if (ImGui::SmallButton("Refresh##FileInspectorRefreshFolders")) {
			//refreshFolderTree();
		}
		ImGui::Separator();
		_fileTreeNodeIndex = 100;
		drawFileTree(_assetTree, _fileTreeNodeIndex, _selectedFileTree);
	}
	ImGui::EndChildFrame();
	ImGui::SameLine();
	ImGui::BeginChildFrame(21, ImVec2());

	ImVec2 itemFrameSize = ImGui::GetCurrentWindow()->Size;

	bool openInspectorPopup = true;
	{// Data
		unsigned int nextFolderId = 200;
		ImGui::Columns(16, NULL, false);
		for (FileTree* subFolder : _selectedFileTree->getChilds()) {
			ImGui::BeginGroup();
			ImGui::PushID(_folderIcon->getTextureId());
			if (ImGui::ImageButton((ImTextureID)nextFolderId++, (ImTextureID)_folderIcon->getTextureId(), { 64, 64 })) {
				_selectedFileTree = subFolder;
			}
			ImGui::PopID();
			ImGui::Text(LatinToAscii(subFolder->getPath().getName()).c_str());
			ImGui::EndGroup();
			ImGui::NextColumn();
			//ImGui::SameLine();
		}
		int ii = 0;
		for (FileData& fileData : _selectedFileTree->getFiles()) {
			void* data = fileData.data;
			if (fileData.type == FileType::Unknown) {
				ImGui::BeginGroup();
				ImGui::Image((ImTextureID)_unloadedIcon->getTextureId(), { 64, 64 });
				ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)ImColor::HSV(1 / 7.0f, 0.6f, 0.6f));
				ImGui::Text(LatinToAscii(fileData.path->getName()).c_str());
				ImGui::PopStyleColor();
				ImGui::EndGroup();
				ImGui::NextColumn();
			} else if(fileData.type == FileType::Scene) {
				ImGui::BeginGroup();
				ImGui::Image((ImTextureID)_sceneIcon->getTextureId(), { 64, 64 });
				ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)ImColor::HSV(1 / 7.0f, 0.6f, 0.6f));
				ImGui::Text(LatinToAscii(fileData.path->getName()).c_str());
				ImGui::PopStyleColor();
				ImGui::EndGroup();
				ImGui::NextColumn();
			} else if (fileData.type == FileType::Texture) {
				ImGui::BeginGroup();
				Texture* tex = static_cast<Texture*>(data);
				ImVec2 tSize = getScaledTexByHeight(tex, 64);
				if (ImGui::ImageButton((ImTextureID)tex->getTextureId(), tSize)) {
					_inspectorMode = FileType::Texture;
					FileTreeQueueItem* dItem = new FileTreeQueueItem();
					dItem->baseTree = _selectedFileTree;
					dItem->data = fileData;
					dItem->useData = true;
					_selectedTexture = dItem;
				}
				if (CreateDraggingObject("Assets:Texture", tex)) {
					ImGui::BeginTooltip();
					ImGui::Image((ImTextureID)tex->getTextureId(), getScaledTexByHeight(tex, 64));
					ImGui::EndTooltip();
				}
				
				if (ImGui::BeginPopupContextItem(("File option menu##" + std::to_string(ii)).c_str())) {
					openInspectorPopup = false;
					if (ImGui::Button(("Copy texture##" + std::to_string(ii)).c_str())) {
						_draggedTexture = tex;
						ImGui::CloseCurrentPopup();
					}
					ImGui::Separator();
					if (ImGui::Button(("Delete File##FileWindowDeleteFile" + std::to_string(ii)).c_str())) {
						FileTreeQueueItem dItem;
						dItem.baseTree = _selectedFileTree;
						dItem.data = fileData;
						dItem.useData = true;
						_assetDeleteQueue.push_back(dItem);
						ImGui::CloseCurrentPopup();
					}
					ImGui::EndPopup();
				}
				if (ImGui::IsItemHovered()) {
					ImGui::BeginTooltip();
					ImGui::Text("Name: %s", fileData.path->getPath().c_str());
					ImGui::Text("Width: %d", tex->getWidth());
					ImGui::Text("Height: %d", tex->getHeight());
					if (_isShiftDown) {
						ImGui::Image((ImTextureID)tex->getTextureId(), getScaledTexByHeight(tex, 512));
					} else {
						ImGui::Text("Hold Y to show bigger view of the texture.");
					}
					ImGui::EndTooltip();
				}
				ImGui::EndGroup();
				ImGui::NextColumn();
			} else if (fileData.type == FileType::Material) {
				ImGui::BeginGroup();
				Material* mat = static_cast<Material*>(data);
				
				ImTextureID matImg = 0;
				Texture* matTex = nullptr;
				if (mat->getPreviewTexture() != nullptr) {
					matImg = (ImTextureID)mat->getPreviewTexture()->getTextureId();
					matTex = mat->getPreviewTexture();
				} else {
					matImg = (ImTextureID)_materialIcon->getTextureId();
					matTex = _materialIcon;
				}

				bool clicked = ImGui::Selectable(("##SelectableMaterial" + std::to_string(ii)).c_str(), _selectedMaterial == mat, 0, ImVec2(0, 64));
				if (CreateDraggingObject("Assets:Material", mat)) {
					ImGui::BeginTooltip();
					ImGui::Image(matImg, getScaledTexByHeight(matTex, 64));
					ImGui::EndTooltip();
				}
				ImGui::SameLine();
				ImGui::Image(matImg, { 64, 64 });
				if (clicked) {
					_selectedMaterial = mat;
					_materialWindowOpened = true;
				}
				if (ImGui::BeginPopupContextItem(("File option menu##" + std::to_string(ii)).c_str())) {
					openInspectorPopup = false;
					if (ImGui::Button(("Copy material##CopyMaterial" + std::to_string(ii)).c_str())) {
						_draggedMaterial = mat;
						ImGui::CloseCurrentPopup();
					}
					ImGui::EndPopup();
				}
				ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)ImColor::HSV(1 / 7.0f, 0.6f, 0.6f));
				ImGui::Text(LatinToAscii(fileData.path->getName()).c_str());
				ImGui::PopStyleColor();
				ImGui::EndGroup();
				if (ImGui::IsItemHovered() && mat->getPreviewTexture() != nullptr) {
					ImGui::BeginTooltip();
					if (_isShiftDown) {
						ImGui::Image((ImTextureID)mat->getPreviewTexture()->getTextureId(), ImVec2(512, 512));
					} else {
						ImGui::Text("Hold Y to show bigger view of the material.");
					}
					ImGui::EndTooltip();
				}
				ImGui::NextColumn();
			} else if (fileData.type == FileType::Object3D) {
				ImGui::BeginGroup();ImGui::BeginGroup();
				Spatial* spatial = static_cast<Spatial*>(data);
				ImGui::ImageButton((ImTextureID)_modelIcon->getTextureId(), { 64, 64 });
				if (ImGui::BeginPopupContextItem(("Model option menu##" + std::to_string(ii)).c_str())) {
					openInspectorPopup = false;
					if (ImGui::Button(("Add to scene##AddToScene" + std::to_string(ii)).c_str())) {
						_selectedSpatial->addChild(spatial->clone());// TODO: This is only for test
						ImGui::CloseCurrentPopup();
					}
					ImGui::EndPopup();
				}
				ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)ImColor::HSV(1 / 7.0f, 0.6f, 0.6f));
				ImGui::Text(LatinToAscii(fileData.path->getName()).c_str());
				ImGui::PopStyleColor();
				ImGui::EndGroup();
				ImGui::NextColumn();
			}
			ii++;
		}
		ImGui::Columns(1);
		/*if (_fileWindowMode == 0) { // TEXTURES
			TextureCacheMap& map = _assetManager->getLoadedTextures();
			int ii = 0;
			int tnSize = 0;
			for (TextureCacheMap::iterator i = map.begin(); i != map.end(); i++) {
				std::string tname = i->first;
				Texture* tex = i->second;
				if (tex->getType() == TextureType::Texture2D) {
					ImVec2 tSize = getScaledTexByHeight(tex, 64);
					if (tnSize > 0 && (tnSize + tSize.x) < itemFrameSize.x) {
						ImGui::SameLine();
					} else {
						tnSize = 0;
					}
					tnSize += tSize.x + 10;
					ImGui::Image((ImTextureID)tex->getTextureId(), tSize);
					if (ImGui::BeginPopupContextItem(("Texture option menu##" + std::to_string(ii)).c_str())) {
						if (ImGui::Button(("Copy texture##" + std::to_string(ii)).c_str())) {
							_draggedTexture = tex;
							ImGui::CloseCurrentPopup();
						}
						ImGui::EndPopup();
					}
					if (ImGui::IsItemHovered()) {
						ImGui::BeginTooltip();
						ImGui::Text("Name: %s", tname.c_str());
						ImGui::Text("Width: %d", tex->getWidth());
						ImGui::Text("Height: %d", tex->getHeight());
						if (_isShiftDown) {
							ImGui::Image((ImTextureID)tex->getTextureId(), getScaledTexByHeight(tex, 512));
						} else {
							ImGui::Text("Hold shift to show bigger view of the texture.");
						}
						ImGui::EndTooltip();
					}
				}
				ii++;
			}
		} else if (_fileWindowMode == 1) {
			
		}*/
	}

	ImGui::EndChildFrame();
	if (openInspectorPopup) {
		bool openNewMaterialPopup = false;
		bool openNewFolderPopup = false;
		if (ImGui::BeginPopupContextItem("File inspector##FileInpectorPopupMenu")) {
			if (ImGui::Button("Create new Material")) {
				openNewMaterialPopup = true;
				ImGui::CloseCurrentPopup();
			}
			if (ImGui::Button("Create folder")) {
				openNewFolderPopup = true;
				ImGui::CloseCurrentPopup();
			}
			if (ImGui::Button("Open in file explorer")) {
				std::string currentPath = SYS::getCurrentDirectory() + "/" + _selectedFileTree->getPath().getPath();
				system(std::string("explorer " + File::fixPath(currentPath, '/', '\\')).c_str());
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		if (openNewFolderPopup) {
			ImGui::OpenPopup("Create folder");
		}
		if (ImGui::BeginPopupModal("Create folder", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::Text("Folder name\n\n");
			ImGui::Separator();
			static char folderName[64];
			ImGui::InputText("", folderName, 64);
			ImGui::Separator();
			if (ImGui::Button("OK##CreateFolderButton", ImVec2(120, 0))) {
				std::string fName(folderName);
				if (fName.length() > 0) {
					FileTree* newTree = _selectedFileTree->getOrCreateDir(fName);
					File newFolder = newTree->getPath();
					if (!newFolder.isExist()) {
						newFolder.mkdirs();
					}
				}
				memset(folderName, 0, 64);
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel##CreateFolderCancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
			ImGui::EndPopup();
		}

		static bool defaultMat = true;

		if (openNewMaterialPopup) {
			ImGui::OpenPopup("Create new material");
			defaultMat = true;
		}
		if (ImGui::BeginPopupModal("Create new material", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::Text("New material name\n\n");
			ImGui::Separator();
			static char materialName[64];
			ImGui::InputText("", materialName, 64);
			ImGui::Separator();
			ImGui::Checkbox("Default shader", &defaultMat);
			static int shader_index = 0;
			if (!defaultMat) {
				ImGui::Text("Select shader");
				ImGui::Combo("##ShaderIndexSelector", &shader_index, _shaderCombo.c_str(), _shadersArrayMap.size());
			}

			ImGui::Separator();
			if (ImGui::Button("OK", ImVec2(120, 0))) {
				if (strlen(materialName) > 0) {
					Material* material = nullptr;
					if (defaultMat) {
						material = _assetManager->loadMaterial("Assets/Materials/PBR_IBL");
					} else {
						material = _assetManager->loadMaterial(_shadersArrayMap[shader_index]->getPath());
					}
					File file(_selectedFileTree->getPath(), std::string(materialName) + ".mat");
					material->setFileSource(file);
					material->setFilename(file.getName());
					FileData data = _selectedFileTree->addData(file, getFileType(file), material);
					_activeProject->getCreatedMaterials().push_back(data);
					memset(materialName, 0, 64);
					ImGui::CloseCurrentPopup();
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
			ImGui::EndPopup();
		}
	}
}

void Editor::initWater() {
	waterY = -20;

	/*_fb = new FrameBuffer(800, 800);
	_fb->createDepthBuffer(ImageFormat::Depth, false);*/

	Material* quadMat = _assetManager->loadMaterial("Assets/Materials/Water");
	_assetTree->addData(File("Memory:Material:WaterMaterial"), FileType::Material, quadMat);
	quadMat->getRenderState().blendMode = BlendMode::Alpha;
	//quadMat->setTexture("ColorMap", _fb->createColorBuffer(ImageFormat::RGBA8, true));
	waterMat = quadMat;
	float qSize = 5000;

	/*GameObject* terrain = new GameObject();
	terrain->setRenderOrder(RenderOrder::Trasparent);
	terrain->setMesh(new Quad(qSize, qSize, false));
	terrain->setRotation(-90, 0, 0);
	terrain->setLocation(-qSize / 2.0f, waterY, qSize / 2.0f);
	terrain->setMaterial(quadMat);
	_mainScene->addChild(terrain);*/

	moveFactor = 0;

	int width = _camera->getWidth();
	int height = _camera->getHeight();

	_waterCamera = new Camera(width, height);
	_waterCamera->setFrustumPerspective(75.0F, (float)_display->getWidth() / (float)_display->getHeight(), 0.1f, 2000.0f);
	_waterCamera->update();


	reflectionFb = new FrameBuffer(width, height);
	reflectionFb->createDepthBuffer(ImageFormat::Depth, false);
	Texture2D* reflectionTex = reflectionFb->createColorBuffer(ImageFormat::RGBA8, true);
	reflectionTex->setWrap(WrapMode::Repeat);
	quadMat->setTexture("ReflectionMap", reflectionTex);

	refractionFb = new FrameBuffer(width, height);
	Texture2D* refractionDepthTex = refractionFb->createDepthBuffer(ImageFormat::Depth, true);
	Texture2D* refractionTex = refractionFb->createColorBuffer(ImageFormat::RGBA8, true);
	refractionDepthTex->setWrap(WrapMode::Repeat);
	refractionTex->setWrap(WrapMode::Repeat);
	quadMat->setTexture("RefractionMap", refractionTex);
	quadMat->setTexture("RefractionDepthMap", refractionDepthTex);

	Texture* dudv = _assetManager->loadTexture("Assets/Materials/Water/Textures/water_dudv.jpg");
	dudv->setWrap(WrapMode::Repeat);
	dudv->setMinMagFilter(MinFilter::Trilinear, MagFilter::Bilinear);
	quadMat->setTexture("DUDVMap", dudv);


	Texture* normalMap = _assetManager->loadTexture("Assets/Materials/Water/Textures/stone-normal.jpg");
	normalMap->setWrap(WrapMode::Repeat);
	normalMap->setMinMagFilter(MinFilter::Trilinear, MagFilter::Bilinear);
	quadMat->setTexture("NormalMap", normalMap);
}

void Editor::renderWater() {
	glEnable(GL_CLIP_DISTANCE0);
	for (Spatial* child : _mainScene->getChilds()) {
		if (child->getName().find_last_of('W') == 0) {
			child->setVisible(false);
			waterY = child->getLocation().y;
		}
	}

	waterMat->setFloat("moveFactor", moveFactor);
	moveFactor += 0.000005f * _timer.getDelta();
	moveFactor = std::fmod(moveFactor, 1.0f);

	float distance = 2 * (_camera->getLocation().y - waterY);
	/*_camera->getLocation().y -= distance;
	_camera->getRotation().x = -_camera->getRotation().x;
	_camera->update();*/

	_waterCamera->setLocation(_camera->getLocation());
	_waterCamera->getLocation().y -= distance;
	_waterCamera->setRotation(_camera->getRotation());
	_waterCamera->getRotation().x = -_waterCamera->getRotation().x;
	_waterCamera->update();

	_viewPort->setCamera(_waterCamera);

	_renderManager->setClipPlane(glm::vec4(0, 1, 0, -waterY + 0.5));
	_renderer->setFrameBuffer(reflectionFb);
	_renderer->clearScreen(true, true, false);
	_renderManager->renderViewPort(_viewPort);

	_viewPort->setCamera(_camera);

	_renderManager->setClipPlane(glm::vec4(0, -1, 0, waterY + 0.5));
	_renderer->setFrameBuffer(refractionFb);
	_renderer->clearScreen(true, true, false);
	_renderManager->renderViewPort(_viewPort);


	_renderer->setFrameBuffer(nullptr);
	glViewport(0, 0, _camera->getWidth(), _camera->getHeight());

	for (Spatial* child : _mainScene->getChilds()) {
		if (child->getName().find_last_of('W') == 0) {
			child->setVisible(true);
		}
	}
	glDisable(GL_CLIP_DISTANCE0);
}

Camera* Editor::getActiveCamera() {
	return _playMode ? _camera : _editorCamera;
}

void Editor::setCameraPlayMode(bool play) {
	_playMode = play;

	Camera* camera = getActiveCamera();

	_viewPort->setCamera(camera);
	_firstPersonControl->setCamera(camera);
	_gizmoControl->setCamera(camera);

	_cloudBatch->getMaterial()->setMatrix4("inv_proj", glm::inverse(camera->getProjectionMatrix()));
	_cloudBatch->getMaterial()->setMatrix4("inv_view", glm::inverse(camera->getViewMatrix()));

	_display->setMouseGrabbed(play);

	_firstPersonControl->_enabled = play;
}

void recompileShaders(Spatial* spatial) {
	for (Spatial* child : spatial->getChilds()) {
		recompileShaders(child);
	}
	if (GameObject* obj = dynamic_cast<GameObject*>(spatial)) {
		Material* mat = obj->getMaterial();
		if (mat != nullptr) {
			mat->getTechnique("Default")->getShader()->compile();
		}
	}
}

ImVec2 operator*(const ImVec2& left, const ImVec2& right) {
	ImVec2 out = left;
	out.x *= right.x;
	out.y *= right.y;
	return out;
}

void renderModuleEditor() {
	ImGui::SetNextWindowSize(ImVec2(500, 500));
	ImGui::Begin("Module editor");

	{
		static ImVec2 scrolling = ImVec2(0.0f, 0.0f);
		static bool show_grid = true;
		bool open_context_menu = false;

		ImGui::Text("Hold middle mouse button to scroll (%.2f,%.2f)", scrolling.x, scrolling.y);
		ImGui::SameLine(ImGui::GetWindowWidth() - 100);
		ImGui::Checkbox("Show grid", &show_grid);
		ImGui::BeginChild("scrolling_region", ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);
		ImGui::PushItemWidth(120.0f);

		ImVec2 offset = scrolling * ImVec2(-1, -1);// = ImGui::GetCursorScreenPos() - scrolling;
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		draw_list->ChannelsSplit(2);

		// Display grid
		if (show_grid) {
			ImU32 GRID_COLOR = ImColor(200, 200, 200, 40);
			float GRID_SZ = 64.0f;
			ImVec2 win_pos = ImGui::GetCursorScreenPos();
			ImVec2 canvas_sz = ImGui::GetWindowSize();
			for (float x = fmodf(offset.x, GRID_SZ); x < canvas_sz.x; x += GRID_SZ) {
				draw_list->AddLine(ImVec2(x, 0.0f) + win_pos, ImVec2(x, canvas_sz.y) + win_pos, GRID_COLOR);
			}
			for (float y = fmodf(offset.y, GRID_SZ); y < canvas_sz.y; y += GRID_SZ)
				draw_list->AddLine(ImVec2(0.0f, y) + win_pos, ImVec2(canvas_sz.x, y) + win_pos, GRID_COLOR);
		}

		draw_list->ChannelsSetCurrent(0);



		draw_list->ChannelsMerge();

		if (!ImGui::IsAnyItemHovered() && ImGui::IsMouseHoveringWindow() && ImGui::IsMouseClicked(1)) {
			open_context_menu = true;
		}

		if (open_context_menu) {
			ImGui::OpenPopup("blueprint_context_menu");
		}

		if (ImGui::BeginPopup("blueprint_context_menu")) {
			ImGui::Text("Modules");
			ImGui::Separator();
			if (ImGui::Button("Add test module")) {
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemActive() && ImGui::IsMouseDragging(2, 0.0f))
			scrolling = scrolling - ImGui::GetIO().MouseDelta;

		ImGui::PopItemWidth();
		ImGui::EndChild();
	}

	ImGui::End();
}

void renderNoiseWindow(Renderer* renderer) {
	static Texture2D* noiseTex = nullptr;
	static glm::vec2 noiseScale = {1.0, 1.0};
	static int currentMode = 0;
	if (noiseTex == nullptr) {
		noiseTex = new Texture2D(128, 128, ImageFormat::RGB8);
		noiseTex->setMinMagFilter(MinFilter::Nearest, MagFilter::Nearest);
		renderer->setTexture(noiseTex, 0);
	}

	ImGui::SetNextWindowSize(ImVec2(250, 0));
	ImGui::Begin("Noise tester", NULL, NULL);

	ImGui::Image((ImTextureID)noiseTex->getTextureId(), ImVec2(128, 128));
	ImGui::Separator();
	bool update = drawVectorInput2("Noise scale", noiseScale);
	update |= ImGui::Combo("Type: ", &currentMode, "Value\0FValueFractal\0Perlin\0PerlinFractal\0Simplex\0SimplexFractal\0Cellular\0WhiteNoise\0Cubic\0CubicFractal\0");
	ImGui::Separator();
	if (ImGui::Button("Regenerate image") || update) {
		if (noiseTex->getDataCount() > 0) {
			delete[] noiseTex->getData(0);
			noiseTex->setData(0, nullptr);
		}

		FastNoise noise;
		noise.SetNoiseType((FastNoise::NoiseType)currentMode);
		unsigned char* pixels = new unsigned char[128 * 128 * 3];

		for (int x = 0; x < 128; x++) {
			for (int y = 0; y < 128; y++) {
				int index = (x + y * 128) * 3;
				int color = 0;

				float noiseVal = noise.GetNoise(x * noiseScale.x, y * noiseScale.y);
				color = (int)((noiseVal / 2.0f + 0.5f) * 0xFF) & 0xFF;

				pixels[index] = color;
				pixels[index+1] = color;
				pixels[index+2] = color;
			}
		}

		noiseTex->setData(pixels);
		noiseTex->setNeedsUpdate(true);
		renderer->setTexture(noiseTex, 0);
	}

	ImGui::End();
}

void Editor::renderImGui() {
	BeginIds();
	//if(true) return;
	_renderer->applyRenderState(DefaultRenderState);
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File", !_playMode)) {
			if (ImGui::MenuItem("Exit")) {
				this->_display->stop();
			}
			/*if(ImGui::MenuItem("Active project: ", "asd", false, false)) {

			}
			if (ImGui::MenuItem("Create new project")) {

			}
			if (ImGui::MenuItem("Load project")) {
				
			}*/
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Object", !_playMode)) {
			if(ImGui::BeginMenu("Add")) {
				if (ImGui::MenuItem("GameObject", "CTRL+G")) {
					if (_selectedSpatial == nullptr) {
						_mainScene->addChild(new GameObject("GameObject"));
					} else {
						_selectedSpatial->addChild(new GameObject("GameObject"));
					}
				}
				ImGui::Separator();
				if (ImGui::MenuItem("Box")) {
					GameObject* obj = new GameObject("Box");
					obj->setMesh(new Box(glm::vec3(0, 0, 0), 0.5, 0.5, 0.5));
					obj->setMaterial(_firstMaterial);
					obj->setFileSource(File("IMesh:Box"));
					if (_selectedSpatial == nullptr) {
						_mainScene->addChild(obj);
					} else {
						_selectedSpatial->addChild(obj);
					}
					_selectedSpatial = obj;
				}
				bool isPlane = false;
				if (ImGui::MenuItem("Quad") || (isPlane = ImGui::MenuItem("Plane"))) {
					GameObject* obj = new GameObject(isPlane ? "Plane" : "Quad");
					obj->setMesh(new Quad(1, 1, false, isPlane));
					obj->setMaterial(_firstMaterial);
					obj->setFileSource(File(isPlane ? "IMesh:Plane" : "IMesh:Quad"));
					if (_selectedSpatial == nullptr) {
						_mainScene->addChild(obj);
					} else {
						_selectedSpatial->addChild(obj);
					}
					_selectedSpatial = obj;
				}
				if (ImGui::MenuItem("Sphere")) {
					GameObject* obj = new GameObject("Sphere");
					obj->setFileSource(File("IMesh:Sphere"));
					obj->setMesh(new Sphere(30, 30, 1, true, false));
					obj->setMaterial(_firstMaterial);
					if (_selectedSpatial == nullptr) {
						_mainScene->addChild(obj);
					} else {
						_selectedSpatial->addChild(obj);
					}
					_selectedSpatial = obj;
				}
				ImGui::Separator();
				if (ImGui::MenuItem("Terrain (Tessellated CDLOD)")) {
					TerrainQuadTree* terrain = new TerrainQuadTree(_viewPort, _assetManager,nullptr);
					if (_selectedSpatial) {
						_mainScene->addChild(terrain);
					} else {
						_selectedSpatial->addChild(terrain);
					}
				}
				ImGui::Separator();
				if (ImGui::MenuItem("Ambient Light", "CTRL+A", false, _selectedSpatial != null)) {
					_mainScene->addLight(new AmbientLight());
				}
				if (ImGui::MenuItem("Directional Light", "CTRL+D", false, _selectedSpatial != null)) {
					_mainScene->addLight(new DirectionalLight());
				}
				if (ImGui::MenuItem("Point Light", "CTRL+P", false, _selectedSpatial != null)) {
					_mainScene->addLight(new PointLight(_camera->getLocation(), makeRGB(255, 255, 255), 1));
				}

				ImGui::EndMenu();
			}
			if (ImGui::MenuItem("Add empty scene", "CTRL+Q")) {
				if (_selectedSpatial == nullptr) {
					_mainScene->addChild(new Spatial("Scene"));
				} else {
					_selectedSpatial->addChild(new Spatial("Scene"));
				}
			}
			if (ImGui::MenuItem("Delete Object", "CTRL+D")) {
				if (_selectedSpatial != nullptr && _selectedSpatial != _mainScene) {
					_selectedSpatial->getParent()->removeChild(_selectedSpatial);
					_selectedSpatial = nullptr;
				}
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Play", !_playMode)) {
			setCameraPlayMode(true);
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Stop", _playMode)) {
			setCameraPlayMode(false);
			ImGui::EndMenu();
		}
		/*if (ImGui::BeginMenu("File")) {
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit")) {
			if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
			if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
			ImGui::Separator();
			if (ImGui::MenuItem("Cut", "CTRL+X")) {}
			if (ImGui::MenuItem("Copy", "CTRL+C")) {}
			if (ImGui::MenuItem("Paste", "CTRL+V")) {}
			ImGui::EndMenu();
		}*/
		ImGui::EndMainMenuBar();
	}
	if(_playMode) return;

	//renderNoiseWindow(_renderer);

	int cBarsSize = getSizePercent(false, 70);
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(250, cBarsSize));
	ImGui::Begin("Scene inspector", NULL, WSTAY);
	IdProvider idProv;
	drawSceneInspector(_mainScene, _selectedSpatial, _selectedMaterial, idProv, _gizmoControl, _inspectorMode);
	ImGui::End();
	if(_draggingSpatial != null) {
		ImVec2 m = ImGui::GetIO().MousePos;
		ImGui::SetNextWindowPos(ImVec2(m.x - 10, m.y));
		ImGui::Begin("1", NULL, ImGuiWindowFlags_Tooltip | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar);
		ImGui::Text(LatinToAscii(_draggingSpatial->getName()).c_str());
		ImGui::End();
	}

	ImGui::SetNextWindowPos(ImVec2(_viewPort->getWidth() - 250, 0));
	ImGui::SetNextWindowSize(ImVec2(250, cBarsSize));
	ImGui::Begin("Inspector", NULL, WSTAY);
	drawInspector(_selectedSpatial, _selectedMaterial, _draggedMaterial, _selectedTexture, _inspectorMode, _materialWindowOpened, _camera);
	ImGui::End();

	ImGui::SetNextWindowPos(ImVec2(0, cBarsSize));
	ImGui::SetNextWindowSize(ImVec2(_viewPort->getWidth(), _viewPort->getHeight() - cBarsSize));
	ImGui::Begin("File inspector", NULL, WSTAY);
	drawFileWindow();
	/*if (ImGui::Button("Recompile shader") && _selectedSpatial != nullptr) {
		recompileShaders(_selectedSpatial);
		_renderer->setInvalidateShader();
	}*/
	ImGui::End();

	//_shadows

	if(_materialWindowOpened) {
		BeginWindow("Material window", 255, 50, 550, 600, false, 0, &_materialWindowOpened);
		drawMaterialWindow();
		EndWindow();
	}

	//renderModuleEditor();

	/*ImGui::SetNextWindowSize(ImVec2(1000, 600), ImGuiCond_FirstUseEver);
	ImGui::Begin("Material editor");
	{
		ImGui::BeginChild("MaterialLeftBar", ImVec2(200, 550), true);
		{
			ImGui::Text("Material list");
			{
				ImGui::BeginChild("Material list", ImVec2(170, 500), true);
				{
					ImGui::Selectable("Sel1");
					ImGui::Selectable("Sel2a");
				}
				ImGui::EndChild();
			}
		}
		ImGui::EndChild();
		ImGui::SameLine();
		ImGui::BeginChild("MaterialRightBar", ImVec2(760, 550), true);
		{

		}
		ImGui::EndChild();
	}
	ImGui::End();*/
	//ImGui::ShowTestWindow();

	/*if (ImGui::TreeNode("Child regions")) {
		ImGui::Text("Without border");
		static int line = 50;
		bool goto_line = ImGui::Button("Goto");
		ImGui::SameLine();
		ImGui::PushItemWidth(100);
		goto_line |= ImGui::InputInt("##Line", &line, 0, 0, ImGuiInputTextFlags_EnterReturnsTrue);
		ImGui::PopItemWidth();
		ImGui::BeginChild("Sub1", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f, 300), false, ImGuiWindowFlags_HorizontalScrollbar);
		for (int i = 0; i < 100; i++) {
			ImGui::Text("%04d: scrollable region", i);
			if (goto_line && line == i)
				ImGui::SetScrollHere();
		}
		if (goto_line && line >= 100)
			ImGui::SetScrollHere();
		ImGui::EndChild();

		ImGui::SameLine();

		ImGui::PushStyleVar(ImGuiStyleVar_ChildWindowRounding, 5.0f);
		ImGui::BeginChild("Sub2", ImVec2(0, 300), true);
		ImGui::Text("With border");
		ImGui::Columns(2);
		for (int i = 0; i < 100; i++) {
			if (i == 50)
				ImGui::NextColumn();
			char buf[32];
			sprintf(buf, "%08x", i * 5731);
			ImGui::Button(buf, ImVec2(-1.0f, 0.0f));
		}
		ImGui::EndChild();
		ImGui::PopStyleVar();

		ImGui::TreePop();
	}*/

	/*ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiCond_FirstUseEver);
	ImGui::Begin("Yoo");
	ImGui::SmallButton("<<");

	ImGui::Text("Filter usage:\n"
		"  \"\"         display all lines\n"
		"  \"xxx\"      display lines containing \"xxx\"\n"
		"  \"xxx,yyy\"  display lines containing \"xxx\" or \"yyy\"\n"
		"  \"-xxx\"     hide lines containing \"xxx\"");

	ImGui::Text("Button");
	ImGui::SameLine();
	if (ImGui::Button("Click")) {
		
	}
	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip("Haloooo");
	}

	ImGui::End();*/

	RestartDragger();

	_uiFocused = ImGui::IsMouseHoveringAnyWindow();
	_gizmoControl->setEnabled(!_uiFocused);
}

void Editor::onMouseMove(int x, int y, int dx, int dy) {
	_paint = _middle;
}

void Editor::onMouseEvent(ButtonEvent e) {
	if (e.type == ButtonType::Right) {
		_firstPersonControl->_enabled = e.state;
		_display->setMouseGrabbed(_firstPersonControl->_enabled);
	} else if (e.state && e.type == ButtonType::Middle) {
		//_terrainTexture->doPaintRadius(e.x, e.y, 100, 0, 0.1);
	} else if (e.state && e.type == ButtonType::Left && !_gizmoControl->isCapturing(e.x, e.y) && !_uiFocused) {
		Ray ray = _camera->getRay(e.x, _camera->getHeight() - e.y);
		CollisionResults results;
		_mainScene->collideWithRay(ray, results);
		if (results.size() > 0) {
			GameObject* obj = results.getClosestCollision().obj;
			_selectedSpatial = obj;
			_gizmoControl->setControl(obj);
			_inspectorMode = FileType::Object3D;
		} else {
			_selectedSpatial = nullptr;
			_gizmoControl->setControl(nullptr);
		}
	}
	if (e.state && e.type == ButtonType::Left) {
		//_fireSound->play();
	}
	_middle = e.state && e.type == ButtonType::Middle;
	if(!e.state) _paint = false;
}

void Editor::onKeyEvent(KeyEvent e) {
	if(e.isText) return;
	_isShiftDown = e.state && e.keyCode == SDLK_y;
	if(!e.state) return;
	if (e.keyCode == SDLK_DELETE) {
		deleteObject(_selectedSpatial);
		_selectedSpatial = nullptr;
		_gizmoControl->setControl(nullptr);
	}
	if (e.keyCode == SDLK_ESCAPE) {
		setCameraPlayMode(false);
	}
}
