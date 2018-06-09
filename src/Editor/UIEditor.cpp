#include "Editor/UIEditor.h"
#include "glm/gtx/string_cast.hpp"
#include "glm/gtc/type_ptr.hpp"

GameObject* createBox(int x, int y, int z, Material* mat) {

}

void UIEditor::init() {
	_viewPort->setBackgroundColor(makeHEX("A1A1A1"));

	UINode* panel = new UINode();
	panel->setSize(200, 350);
	panel->setLocation(100, 100);
	RectangleRendeer* recR = new RectangleRendeer();
	panel->addRenderer(recR, 0);

	ScrollPane* scrollPane = new ScrollPane(panel, true, false);


	/*UINode* panelIn = new UINode();
	panelIn->setSize(60, 25);
	panelIn->setLocation(0, 0);
	panelIn->addRenderer(new RectangleRendeer(), 0);
	scrollPane->addChild(panelIn);*/

	Button* btn = new Button("TestBtn");
	scrollPane->addChild(btn);
	//_uiNode->addChild(panel);

	{// SKY
		_cloudBatch = new SpriteBatch(_guiCamera, _renderManager, _assetManager->loadMaterial("Assets/Materials/Clouds"));
		_cloudBatch->getMaterial()->setVector2("view_port", glm::vec2(_camera->getWidth(), _camera->getHeight()));
		_cloudBatch->getMaterial()->setMatrix4("inv_proj", glm::inverse(_camera->getProjectionMatrix()));
		time = 0;
	}

	GameObject* planeObj = new GameObject;
	planeObj->setMesh(new Quad(200, 200, false));
	TangentGenerator::generate(planeObj->getMesh(), true, false);
	
	mat = _assetManager->loadMaterial("Assets/Materials/PBR");
	//mat->setVector3("FogColor", glm::vec3(1, 1, 1));
	//mat->setVector3("lightPos", glm::vec3(0.5f, 0.5f, 0.5f));
	mat->getRenderState().faceCull = FaceCull::Off;

	glm::vec3 lightPositions[] = {
		glm::vec3(-10.0f,  10.0f, 10.0f),
		glm::vec3(10.0f,  10.0f, 10.0f),
		glm::vec3(-10.0f, -10.0f, 10.0f),
		glm::vec3(10.0f, -10.0f, 10.0f),
	};
	glm::vec3 lightColors[] = {
		glm::vec3(300.0f, 300.0f, 300.0f),
		glm::vec3(300.0f, 300.0f, 300.0f),
		glm::vec3(300.0f, 300.0f, 300.0f),
		glm::vec3(300.0f, 300.0f, 300.0f)
	};

	for (unsigned int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i) {
		mat->setVector3("lightPositions[" + std::to_string(i) + "]", lightPositions[i]);
		mat->setVector3("lightColors[" + std::to_string(i) + "]", lightColors[i]);
	}

	Material* parl = _assetManager->loadMaterial("Assets/Materials/Lighting");
	//parl->getRenderState().faceCull = FaceCull::Off;
	parl->setUseLight(true);
	parl->setFloat("TextureScale", 50.0f);
	
	//parl->setAmbientColor(makeRGB(55, 55, 55));

	//parl->setDiffuseMap(_assetManager->loadTexture("Assets/Textures/moss/color.jpg"));
	//parl->setNormalMap(_assetManager->loadTexture("Assets/Textures/moss/normal.png"));
	//parl->setSpecularMap(_assetManager->loadTexture("Assets/Textures/needles_grass2/needles_grass2_Glossiness.jpg"));
	//parl->setShininess(75.0f);

	planeObj->setMaterial(parl);
	planeObj->setLocation(-100, 0, 100);
	planeObj->setRotation(-90, 0, 0);

	_mainScene->addLight(new AmbientLight(makeRGB(0, 0, 0)));
	
	dirLight = new DirectionalLight(makeRGB(200, 200, 200), glm::vec3(-0.5, -0.5, -0.5));
	light = new PointLight(glm::vec3(10, 0, 10), makeRGB(100, 100, 200), 20);

	_mainScene->addLight(dirLight);
	//_mainScene->addLight(light);

	_mainScene->addChild(planeObj);

	_camera->setLocation(0, 2.5f, 0);
	_firstPersonControl->_flyMode = false;

	/*{
		Material* l2 = _assetManager->loadMaterial("Assets/Materials/Lighting");
		l2->setUseLight(true);

		l2->setDiffuseMap(_assetManager->loadTexture("Assets/Textures/wall/cihly_BaseColor.tga"));
		l2->setNormalMap(_assetManager->loadTexture("Assets/Textures/wall/cihly_Normal.tga"));

		l2->setFloat("TextureScale", 10.0f);

		GameObject* box = new GameObject();
		box->setMesh(new Quad(20, 20, false));
		TangentGenerator::generateTangentAndBitangents(box->getMesh());
		box->setMaterial(l2);

		box->setLocation(5, -5, 5);

		_mainScene->addChild(box);
	}*/

	{
		Spatial* tree = _assetManager->loadModel("C:\\Users\\Sam\\Desktop\\white_oak\\white_oak.obj");
		tree->setScale(0.05, 0.05, 0.05);
		tree->setLocation(10, 0, 20);
		_mainScene->addChild(tree);
	}

	doors.push_back(new DoorObject(0, 2, 0, mat));
	doors.push_back(new DoorObject(2, 2, 0, mat));

	for (DoorObject* door : doors) {
		door->addToScene(_mainScene);
	}

	_firstPersonControl->_enabled = false;
	_display->setMouseGrabbed(false);
	_mouseState = false;

	_firstPersonControl->_enabled = false;
	//_thirdPersonControl->setEnabled(true);

	/*Spatial* _player = _assetManager->loadModel("Assets/Models/Player/model.dae");
	_mainScene->addChild(_player);*/

	/*GameObject* obj = (GameObject*)_assetManager->loadModel("C:\\Users\\Sam\\Desktop\\plant\\zidan.obj")->getChilds()[0];
	Material* mat = obj->getMaterial();
	mat->setVector3("Bloom", glm::vec3(1, 1, 1));
	mat->setAmbientColor(makeRGB(0, 0, 0));
	mat->setUseLight(true);
	//mat->getRenderState().faceCull = FaceCull::Off;
	mat->setBool("UseAlphaMap", true);
	Texture* alpha = _assetManager->loadTexture("C:\\Users\\Sam\\Desktop\\plant\\rostlina_alfa.png");
	alpha->setMinMagFilter(MinFilter::Nearest, MagFilter::Nearest);
	mat->setTexture("AlphaMap", alpha);
	mat->setNormalMap(_assetManager->loadTexture("C:\\Users\\Sam\\Desktop\\plant\\rostlina_norm.png"));
	obj->setScale(0.003, 0.003, 0.003);
	_mainScene->addChild(obj);

	std::vector<glm::mat4> pos;

	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			float x = i * 1;
			float z = j * 1;

			x += (rand() % 10);
			z += (rand() % 10);

			float yr = glm::abs(rand() % 360);

			obj->setLocation(x, 0, z);
			obj->setRotation(0, yr, 0);
			//sp->setLocation(0, 0, 0);

			pos.push_back(obj->getModelMatrix());
		}
	}

	FloatBuffer* iData = FloatBuffer::allocate(16 * pos.size());

	for (glm::mat4 p : pos) {
		const float* f = (const float*)glm::value_ptr(p);
		for (int i = 0; i < 16; i++) {
			iData->put(f[i]);
		}
	}

	obj->getMaterial()->setBool("UseInstancing", true);

	obj->getMesh()->setData(MeshBuffer::Type::InstanceData, 16, iData);
	obj->getMesh()->setInstanceCount(pos.size());
	*/
	{
		std::vector<glm::vec3> pp;
		for (int i = -100; i < 100; i++) {
			for (int j = -100; j < 100; j++) {
				float x = i * 0.8f;
				float z = j * 0.8f;

				x += (rand() % 10);
				z += (rand() % 10);
				pp.push_back(glm::vec3(x, 0, z));
			}
		}

		FloatBuffer* positions = FloatBuffer::allocate(pp.size() * 3);
		for (glm::vec3 p : pp) {
			positions->put(p.x)->put(p.y)->put(p.z);
		}

		Mesh* grassMesh = new Mesh();
		grassMesh->setData(MeshBuffer::Type::Position, 3, positions);
		grassMesh->setMode(Mode::Points);

		GameObject* grs = new GameObject();
		grs->setRenderOrder(RenderOrder::Trasparent);
		//grs->setScale(0.25, 0.25, 0.25);
		grs->setMesh(grassMesh);

		grsMat = _assetManager->loadMaterial("Assets/Materials/Grass");
		grsMat->getRenderState().faceCull = FaceCull::Off;
		grsMat->getRenderState().blendMode = BlendMode::Alpha;
		Texture* gtex = _assetManager->loadTexture("Assets/Materials/Grass/Textures/edited.png");
		gtex->setWrap(WrapMode::ClampToEdge);
		grsMat->setTexture("gSampler", gtex);

		grs->setMaterial(grsMat);
		_mainScene->addChild(grs);
	}

	//_bloomFilter = new PostBloom(_assetManager, _viewPort, _guiCamera, _renderManager);*/

	_inspectorMode = -1;

	_display->setFileDropListener([this](std::string file, int x, int y) {
		File f(file);
		std::string exp = f.getExtension();
		if (exp == "png" || exp == "jpg" || exp == "jpeg" || exp == "gif" || exp == "tiff") {

		} else {
			Spatial* model = _assetManager->loadModel(file);
			if (model != nullptr) {
				_mainScene->addChild(model);
			}
		}
	});

	_shadows = new ShadowPost(dirLight, _assetManager, _viewPort, _camera, _renderManager);
}

void UIEditor::update() {
	_camera->update();
	for (DoorObject* door : doors) {
		door->update(_timer.getDelta());
	}
	//light->setPosition(_camera->getLocation());
	//mat->setVector3("lightPositions[0]", _camera->getLocation());
}

#include "glm/gtx/quaternion.hpp"

void drawBoundsa(BoundingBox* bb) {
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

void UIEditor::render() {
	_renderer->clearColor(_viewPort->getBackgroundColor());
	_renderer->clearScreen(true, true, false);

	time += 0.0001 * _timer.getDelta();
	glm::vec3 lightPos = glm::vec3(glm::cos(time * 0.05f) * 100, glm::sin(time * 0.05f) * 100, glm::sin(M_PI / 2 + time * 0.05f) * 100);
	glm::vec3 lightDir = glm::normalize(lightPos);

	dirLight->setDirection(lightDir * glm::vec3(-1, -1, -1));

	//grsMat->setFloat("fTimePassed", time * 10);

	_cloudBatch->getMaterial()->setVector3("LightDir", lightDir);
	_cloudBatch->getMaterial()->setFloat("iGlobalTime", time);
	_cloudBatch->getMaterial()->setMatrix4("inv_view", glm::inverse(_camera->getViewMatrix()));
	_cloudBatch->drawTexture(nullptr, 0, 0, _camera->getWidth(), _camera->getHeight());

	_shadows->render();

	_renderManager->renderViewPort(_viewPort);
	//_bloomFilter->render();

	glBindVertexArray(0);
	glUseProgram(0);
	glDisable(GL_TEXTURE_2D);
	glLoadIdentity();
	glLoadMatrixf(&_camera->getProjectionViewMatrix()[0][0]);
	glLineWidth(1.0f);

	/*Ray ray = _camera->getRay(_inputManager->getMouseX(), _camera->getHeight() - _inputManager->getMouseY());
	CollisionResults results;
	for (DoorObject* door : doors) {
		door->_obj->collideWithRay(ray, results);
	}
	collisions = results.size();
	if (results.size() > 0) {
		CollisionResult r = results.getClosestCollision();
		ress = r;

		glColor3f(1, 0, 0);
		glLineWidth(2);
		glBegin(GL_LINES);
		{
			glVertex3f(r.contactPoint.x, r.contactPoint.y, r.contactPoint.z);
			glVertex3f(r.contactPoint.x + r.contactNormal.x * 100, r.contactPoint.y + r.contactNormal.y * 100, r.contactPoint.z + r.contactNormal.z * 100);
		}
		glEnd();
	}

	for (DoorObject* door : doors) {
		BoundingBox* worldBB = door->_obj->getMesh()->getBounds()->transform(door->_obj->getLocation(), door->_obj->getRotation(), glm::vec3(1, 1, 1));
		drawBoundsa(worldBB);
		delete worldBB;
	}*/

	/*

	Ray r = rayToDraw;

	door->_obj->setRotation(0, 90, 0);
	glm::mat4& mm = door->_obj->getModelMatrix();
	door->_obj->setRotation(0, 0, 0);

	glm::mat4 inv = glm::inverse(mm);

	glm::quat rotation = glm::toQuat(mm);

	r.origin = mm * glm::vec4(r.origin, 1.0f);
	r.direction = rotation * r.direction;

	glColor3f(1, 0, 0);
	glLineWidth(2);
	glBegin(GL_LINES);
	{
		glVertex3f(r.origin.x, r.origin.y, r.origin.z);
		glVertex3f(r.origin.x + r.direction.x * 100, r.origin.y + r.direction.y * 100, r.origin.z + r.direction.z * 100);
	}
	glEnd();*/

	glEnable(GL_TEXTURE_2D);
}

void renderSpatialInspectorUI(Spatial* obj) {
	GameObject* objInst = nullptr;
	if (objInst = dynamic_cast<GameObject*>(obj)) {
		ImGui::Text("GameObject - %s", obj->getName());
	} else {
		ImGui::Text("Scene - %s", obj->getName());
	}
	ImGui::Separator();

	ImGui::Text("Location");
	ImGui::DragFloat("X##Loc", &obj->getLocation().x, 0.5f);
	ImGui::DragFloat("Y##Loc", &obj->getLocation().y, 0.5f);
	ImGui::DragFloat("Z##loc", &obj->getLocation().z, 0.5f);

	ImGui::Text("Rotation");
	{
		ImGui::SameLine();
		if (ImGui::Button("Reset##RotationReset")) {
			obj->setRotation(0, 0, 0);
		}
	}
	ImGui::DragFloat("X##Rot", &obj->getRotation().x, 0.5f);
	ImGui::DragFloat("Y##Rot", &obj->getRotation().y, 0.5f);
	ImGui::DragFloat("Z##Rot", &obj->getRotation().z, 0.5f);

	ImGui::Text("Scale");
	{
		ImGui::SameLine();
		if (ImGui::Button("Reset##ScaleReset")) {
			obj->setScale(1, 1, 1);
		}
	}
	ImGui::DragFloat("X##Scale", &obj->getScale().x, 0.5f);
	ImGui::DragFloat("Y##Scale", &obj->getScale().y, 0.5f);
	ImGui::DragFloat("Z##Scale", &obj->getScale().z, 0.5f);

	if(objInst == nullptr) return;

	ImGui::Separator();

	Mesh* mesh = objInst->getMesh();
	ImGui::Text("Mesh: %s", mesh != nullptr ? "Yes" : "No");
	if (mesh != nullptr) {
		ImGui::Text("Collision: %s", mesh->getCollider() != nullptr ? "Yes" : "No");
	}

	Material* mat = objInst->getMaterial();

	ImGui::Separator();
	ImGui::Text("Material: %s", mat != nullptr ? mat->getName() : "None");
}

void drawSceneUI(Spatial* sp, int i, int* iId, UIEditor* editor) {
	std::string name = sp->getName();
	if (name.length() == 0) {
		name = "Unknown";
	}
	if (ImGui::TreeNodeEx((name + "##Node" + std::to_string(i)).c_str(), ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_Leaf)) {
		if (ImGui::IsItemClicked()) {
			*iId = 0;
			editor->_selectedObj = sp;
		}
		for (Spatial* child : sp->getChilds()) {
			drawSceneUI(child, i++, iId, editor);
		}
		ImGui::TreePop();
	}
}

void UIEditor::renderUI() {
	Engine::renderUI();

	_uiRenderer->drawText("FPS: " + std::to_string((int)_timer.getFrameRate()), 10, _camera->getHeight()-20, 16, makeRGB(20, 20, 20));

	_uiRenderer->drawText("Location: " + glm::to_string(_camera->getLocation()), 10, 10, 16, makeRGB(20, 20, 20));
	_uiRenderer->drawText("MouseCols: " + std::to_string(collisions), 10, 25, 16, makeRGB(20, 20, 20));
	_uiRenderer->drawText("CPos: " + glm::to_string(ress.contactPoint), 10, 25 + 15, 16, makeRGB(20, 20, 20));
	_uiRenderer->drawText("CNorm: " + glm::to_string(ress.contactNormal), 10, 25 + 15 + 15, 16, makeRGB(20, 20, 20));
}

void UIEditor::renderImGui() {
	{
		if (_inspectorMode >= 0) {
			ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiCond_FirstUseEver);
			bool b = true;
			ImGui::Begin("Inspector", &b, 0/*ImGuiWindowFlags_ChildWindowAutoFitX ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/);
			{
				if (_inspectorMode == 0 && _selectedObj != nullptr) {
					renderSpatialInspectorUI(_selectedObj);
				}
			}
			ImGui::End();
		}

		{
			bool b = true;
			ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiCond_FirstUseEver);
			ImGui::Begin("Scene", &b, ImGuiWindowFlags_AlwaysAutoResize);
			ImGui::Text("Scene tree");
			ImGui::Separator();
			drawSceneUI(_mainScene, 0, &_inspectorMode, this);
			ImGui::End();
		}
		ImGui::Begin("Test windows");
		ImGui::Text("Hello, world!");

		ImGui::SliderFloat("float", &time, 0.0f, 100.0f);
		if (ImGui::Button("Reset")) {
			time = 0;
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Resets a time");
		ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)ImColor::HSV(1.0f / 7.0f, 0.6f, 0.6f));
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::PopStyleColor(1);
		ImGui::PopID();
		if (ImGui::BeginMenu("Yo menu")) {

			ImGui::MenuItem("TestItem");

			ImGui::EndMenu();
		}
		if (ImGui::IsItemHovered()) {
			ImGui::BeginTooltip();
			ImGui::Text("I am a fancy tooltip");
			static float arr[] = { 0.6f, 0.1f, 1.0f, 0.5f, 0.92f, 0.1f, 0.2f, 10 };
			ImGui::PlotLines("Curve", arr, 8);
			ImGui::EndTooltip();
		}
		ImGui::Image((ImTextureID)_shadows->getDepthTexture()->getTextureId(), ImVec2(128, 128));
		ImGui::End();

		{
			ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiCond_FirstUseEver);
			ImGui::Begin("Textures");
			{
				TextureCacheMap& map = _assetManager->getLoadedTextures();
				for (TextureCacheMap::iterator i = map.begin(); i != map.end(); i++) {
					std::string tname = i->first;
					Texture* tex = i->second;
					if (tex->getType() == TextureType::Texture2D) {
						ImGui::Image((ImTextureID)tex->getTextureId(), ImVec2(128, 128));
						if (ImGui::IsItemHovered()) {
							ImGui::BeginTooltip();
							ImGui::Text("Name: %s", File(tname).getName());
							ImGui::Text("Width: %d", tex->getWidth());
							ImGui::Text("Height: %d", tex->getHeight());
							ImGui::EndTooltip();
						}
					}
				}
			}
			ImGui::End();
		}
	}
}

void UIEditor::onMouseMove(int x, int y, int dx, int dy) {
	
}

void UIEditor::onMouseEvent(ButtonEvent e) {
	if (e.state) {
		if (e.type == ButtonType::Left && !ImGui::IsMouseHoveringAnyWindow()) {
			Ray ray = _camera->getRay(e.x, _camera->getHeight() - e.y);
			CollisionResults results;
			if (_mainScene->collideWithRay(ray, results)) {
				_inspectorMode = 0;
				_selectedObj = results.getClosestCollision().obj;
			} else {
				_inspectorMode = -1;
			}
			/*Ray ray;
			for (DoorObject* door : doors) {
				if (_mouseState) {
					//ray = door->doOpenClose(_camera->getWidth() / 2, _camera->getHeight() / 2, doors, _camera);
				} else {
					//ray = door->doOpenClose(e.x, _camera->getHeight() - e.y, doors, _camera);
				}
			}
			rayToDraw = ray;*/
		} else if (e.type == ButtonType::Right) {
			_mouseState = !_mouseState;
			_firstPersonControl->_enabled = _mouseState;
			_display->setMouseGrabbed(_mouseState);
		}
	}
}

void UIEditor::onKeyEvent(KeyEvent e) {
	if(e.isText) return;
	if(!e.state) return;
	if (e.keyCode == SDLK_e) {
		if (_mouseState) {
			for (DoorObject* door : doors) {
				door->doOpenClose(_camera->getWidth() / 2, _camera->getHeight() / 2, doors, _camera);
			}
		}
	}
}
