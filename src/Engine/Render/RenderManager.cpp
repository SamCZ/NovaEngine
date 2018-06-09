#include "Engine/Render/RenderManager.h"
#include <iostream>
#include "Engine/Loaders/AssetManager.h"
#include "Engine/Post/PostFilter.h"
#include "Engine/Render/SpriteBatch.h"

namespace NovaEngine {
	RenderManager::RenderManager(Renderer* renderer) : _viewPorts(), _forcedTechnique(), _forcedTechniqueDef(nullptr) {
		_renderer = renderer;
		_clipPlane = glm::vec4(0);
		_shadowMap = nullptr;

		_isPBRIBL = false;

		_baseRender = nullptr;
	}

	RenderManager::~RenderManager() {
		for (ViewPort* viewPort : _viewPorts) {
			delete viewPort;
		}
		_viewPorts.clear();
	}

	void RenderManager::initPBR_IBL(AssetManager* assetManager) {
		std::cout << "Initializing hdr..." << std::endl;
		Texture* hdrMap = assetManager->loadHDR("Assets/Textures/hdr/HDR_029_Sky_Cloudy_Ref.hdr");

		if (hdrMap != nullptr) {
			std::cout << "HRD Map OK." << std::endl;

			_envMap = _renderer->createPBRMapFromHDR(assetManager, hdrMap, PBRMapType::EnviromentFromHDR);
			if (_envMap != nullptr) {
				std::cout << "ENV Map OK." << std::endl;

				_irradianceMap = _renderer->createPBRMapFromHDR(assetManager, _envMap, PBRMapType::IrradianceFromEnviroment);
				if (_irradianceMap != nullptr) {
					std::cout << "Irradiance Map OK." << std::endl;

					_prefilteredMap = _renderer->createPBRMapFromHDR(assetManager, _envMap, PBRMapType::PrefilteredMapFromEnviroment);
					if (_prefilteredMap != nullptr) {
						_brdfLLUT = _renderer->createBRDFLUT(assetManager);
						//_brdfLLUT = assetManager->loadTexture("Assets/Textures/ibl_brdf_lut.png");
						if (_brdfLLUT != nullptr) {
							std::cout << "PBR IBL Loaded." << std::endl;
							_isPBRIBL = true;
						}
					}
				}
			}
		}
		std::cout << "Hdr init end..." << std::endl;
		_renderer->setFrameBuffer(nullptr);
	}

	TextureCubeMap* RenderManager::getEnviromentMap() {
		return _envMap;
	}

	ViewPort* RenderManager::createViewPort(std::string name, Camera* camera, bool forcedSize) {
		ViewPort* viewPort = new ViewPort(name, camera);
		viewPort->setForcedSize(forcedSize);
		Spatial* scene = new Spatial(name + "Scene");
		viewPort->setScene(scene);
		_viewPorts.push_back(viewPort);
		return viewPort;
	}

	ViewPort* RenderManager::getViewPort(std::string name) {
		for (ViewPort* wp : _viewPorts) {
			if (wp->getName() == name) {
				return wp;
			}
		}
		return nullptr;
	}

	void RenderManager::resize(int w, int h) {
		//glViewport(0, 0, w, h);
		for (ViewPort* viewPort : _viewPorts) {
			if (!viewPort->isForcedSize()) {
				Camera* camera = viewPort->getCamera();
				if (camera != nullptr) {
					camera->resize(w, h);
				}
			}
		}
	}

	void RenderManager::renderViewPortWithFilters(ViewPort* viewPort, SpriteBatch* spriteBatch, AssetManager* assetManager) {
		if (_baseRender == nullptr) {
			_baseRender = new PostFilterMesh(viewPort->getCamera(), this, false);
			_baseTexture = _baseRender->addColorTexture(ImageFormat::RGBA8);
			_baseBloomTexture = _baseRender->addColorTexture(ImageFormat::RGBA8);
			_baseDepthTexture = _baseRender->addColorTexture(ImageFormat::RGB32F);
			_baseNormalTexture = _baseRender->addColorTexture(ImageFormat::RGBA8);

			_baseBlur = new PostBlur(assetManager, getViewPort("ViewPortGui")->getCamera(), this);
			_baseBlur->setInput(_baseBloomTexture);

			Material* postMat = assetManager->loadMaterial("Assets/Materials/PostFilter");
			_basePostRender = new PostFilter2D(postMat, getViewPort("ViewPortGui")->getCamera(), this);
			_basePostRender->setDefaultOutput();
			_basePostRender->setTexture("SceneMap", _baseTexture);
			_basePostRender->setTexture("BloomMap", _baseBlur->getResult());
		}
		_baseRender->render(viewPort, "Default");
		_baseBlur->render();
		_basePostRender->render();

		spriteBatch->drawTexture(_baseDepthTexture, 0, 0, viewPort->getWidth(), viewPort->getHeight(), makeRGB(255, 255, 255), true, true);
		//spriteBatch->drawTexture(_baseNormalTexture, 0, 0, viewPort->getWidth(), viewPort->getHeight(), makeRGB(255, 255, 255), true, true);
	}

	void RenderManager::renderViewPort(ViewPort* viewPort) {
		if (viewPort != nullptr) {
			Camera* camera = viewPort->getCamera();
			Spatial* scene = viewPort->getScene();
			glViewport(0, 0, camera->getWidth(), camera->getHeight());

			if (camera != nullptr && scene != nullptr) {


				renderSpatial(scene, camera, RenderOrder::Opaque);
				renderSpatial(scene, camera, RenderOrder::Sky);
				renderSpatial(scene, camera, RenderOrder::Trasparent);
				renderSpatial(scene, camera, RenderOrder::Gui);

				//renderPreparedObjs();

				//_objToRender.clear();
			}
		}
	}

	void RenderManager::renderPreparedObjs() {
		for (GameObject* obj : _objToRender) {
			_renderer->applyRenderState(obj->getMaterial()->getRenderState());
			obj->getMaterial()->setMatrix4("ModelMatrix", obj->getModelMatrix());
			obj->preRender();
			obj->getMaterial()->render(_renderer, obj->getMesh());
		}
	}

	void RenderManager::prepareRenderSpatial(Spatial* spatial, Camera* camera, RenderOrder order) {
		if (spatial->isLightNode()) {
			_lightsToRender.clear();
		}
		for (Light* light : spatial->getLights()) {
			_lightsToRender.push_back(light);
		}
		for (Spatial* child : spatial->getChilds()) {
			prepareRenderSpatial(child, camera, order);
		}

		if (spatial->getType() == SpatialType::GameObject) {
			GameObject* obj = static_cast<GameObject*>(spatial);
			if (obj->getRenderOrder() == order && obj->isVisibleInTree()) {
				prepareRenderGameObject(obj, camera);
			}
		}
	}

	void RenderManager::prepareRenderGameObject(GameObject* obj, Camera* camera) {
		Mesh* mesh = obj->getMesh();
		Material* material = obj->getMaterial();
		if (mesh != nullptr && material != nullptr) {
			TechniqueDef* defaultTech = material->getTechnique("Default");
			if (_forcedTechnique.length() > 0) {
				TechniqueDef* tech = material->getTechnique(_forcedTechnique);
				if (tech != nullptr) {
					defaultTech = tech;
				} else {
					return;
				}
			}
			if (_forcedTechniqueDef != nullptr) {
				defaultTech = _forcedTechniqueDef;
			}

			glm::mat4 mvp = camera->getProjectionViewMatrix() * obj->getModelMatrix();
			material->setTechnique(defaultTech);
			material->setCamera(camera);
			material->setMatrix4("ModelMatrix", obj->getModelMatrix());
			material->setMatrix4("ModelViewProjectionMatrix", mvp);
			material->setMatrix4("MVP", mvp);
			material->setVector4("ClipPlane", _clipPlane);
			material->setVector3("ViewPos", _viewPorts[0]->getCamera()->getLocation());

			material->setMatrix4("ModelViewMatrix", camera->getViewMatrix() * obj->getModelMatrix());
			material->setMatrix4("ViewProjectionMatrix", camera->getProjectionViewMatrix());

			static glm::mat4 lightProj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 500.0f);

			material->setMatrix4("lightProjection", lightProj);

			if (_forcedTechnique == "Shadows") {
				material->getRenderState().faceCull = FaceCull::Back;
			}

			if (_isPBRIBL && material->getName() == "PBR_IBL") {
				material->setTexture("IrradianceMap", _irradianceMap, false);
				material->setTexture("PrefilterMap", _prefilteredMap, false);
				material->setTexture("BrdfLUT", _brdfLLUT, false);
			}

			//mat4 normalMatrix = transpose(inverse(modelView));

			glm::mat3 worldNormalMatrix = glm::transpose(glm::inverse(glm::mat3(obj->getModelMatrix())));
			glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(camera->getViewMatrix() * obj->getModelMatrix())));


			material->setMatrix3("WorldNormalMatrix", worldNormalMatrix);
			material->setMatrix3("NormalMatrix", normalMatrix);

			if (_shadowMap != nullptr) {
				material->setTexture("ShadowMap", _shadowMap);
				material->setMatrix4("ShadowMatrix", _shadowMatrix);
				material->setBool("UseShadows", true);
			} else {
				material->setBool("UseShadows", false);
			}
			material->setLights(_lightsToRender);
			//_renderer->applyRenderState(material->getRenderState());
			//obj->preRender();
			material->uploadParamsToGPU(_renderer);
			_objToRender.push_back(obj);
		}
	}

	void RenderManager::renderSpatial(Spatial* spatial, Camera* camera, RenderOrder order) {
		if (spatial->isLightNode()) {
			_lightsToRender.clear();
		}
		for (Light* light : spatial->getLights()) {
			_lightsToRender.push_back(light);
		}
		for (Spatial* child : spatial->getChilds()) {
			renderSpatial(child, camera, order);
		}

		if (spatial->getType() == SpatialType::GameObject) {
			GameObject* obj = static_cast<GameObject*>(spatial);
			if (obj->getRenderOrder() == order && obj->isVisibleInTree()) {
				renderGameObject(obj, camera);
			}
		}
	}

	void RenderManager::renderGameObject(GameObject* obj, Camera* camera) {
		Mesh* mesh = obj->getMesh();
		Material* material = obj->getMaterial();
		if (mesh != nullptr && material != nullptr) {
			TechniqueDef* defaultTech = material->getTechnique("Default");
			if (_forcedTechnique.length() > 0) {
				TechniqueDef* tech = material->getTechnique(_forcedTechnique);
				if (tech != nullptr) {
					defaultTech = tech;
				} else {
					return;
				}
			}
			if (_forcedTechniqueDef != nullptr) {
				defaultTech = _forcedTechniqueDef;
			}

			glm::mat4 mvp = camera->getProjectionViewMatrix() * obj->getModelMatrix();
			material->setTechnique(defaultTech);
			material->setCamera(camera);
			material->setMatrix4("ModelMatrix", obj->getModelMatrix());
			material->setMatrix4("ModelViewProjectionMatrix", mvp);
			material->setMatrix4("MVP", mvp);
			material->setVector4("ClipPlane", _clipPlane);
			material->setVector3("ViewPos", _viewPorts[0]->getCamera()->getLocation());

			material->setMatrix4("ModelViewMatrix", camera->getViewMatrix() * obj->getModelMatrix());
			material->setMatrix4("ViewProjectionMatrix", camera->getProjectionViewMatrix());

			static glm::mat4 lightProj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 500.0f);

			material->setMatrix4("lightProjection", lightProj);

			/*if (_forcedTechnique == "Shadows") {
				material->getRenderState().faceCull = FaceCull::Back;
			}*/

			if (_isPBRIBL && material->getName() == "PBR_IBL") {
				material->setTexture("IrradianceMap", _irradianceMap, false);
				material->setTexture("PrefilterMap", _prefilteredMap, false);
				material->setTexture("BrdfLUT", _brdfLLUT, false);
			}

			//mat4 normalMatrix = transpose(inverse(modelView));

			glm::mat3 worldNormalMatrix = glm::transpose(glm::inverse(glm::mat3(obj->getModelMatrix())));
			glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(camera->getViewMatrix() * obj->getModelMatrix())));


			material->setMatrix3("WorldNormalMatrix", worldNormalMatrix);
			material->setMatrix3("NormalMatrix", normalMatrix);

			/*if (_shadowMap != nullptr) {
				material->setTexture("ShadowMap", _shadowMap);
				material->setMatrix4("ShadowMatrix", _shadowMatrix);
				material->setBool("UseShadows", true);
			} else {
				material->setBool("UseShadows", false);
			}*/
			material->setLights(_lightsToRender);
			_renderer->applyRenderState(material->getRenderState());
			obj->preRender();
			material->render(_renderer, mesh);
		}
	}

	void RenderManager::addPostProcess(PostProcess* post) {
		if (!havePostProcess(post)) {
			_postProcessors.push_back(post);
		}
	}

	void RenderManager::removePostProcess(PostProcess* post) {
		if (havePostProcess(post)) {
			_postProcessors.erase(std::find(_postProcessors.begin(), _postProcessors.end(), post));
		}
	}

	bool RenderManager::havePostProcess(PostProcess* post) {
		return std::find(_postProcessors.begin(), _postProcessors.end(), post) != _postProcessors.end();
	}

	void RenderManager::setForcedTechnique(std::string technique) {
		_forcedTechnique = technique;
	}

	void RenderManager::setForcedTechniqueDef(TechniqueDef* technique) {
		_forcedTechniqueDef = technique;
	}

	void RenderManager::setClipPlane(glm::vec4 plane) {
		_clipPlane = plane;
	}

	Renderer* RenderManager::getRenderer() {
		return _renderer;
	}

	void RenderManager::setShadowMap(Texture* tex) {
		_shadowMap = tex;
	}

	void RenderManager::setShadowMatrix(glm::mat4 m) {
		_shadowMatrix = m;
	}
}