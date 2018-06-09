#include "Engine/Material/Material.h"
#include "glm/gtc/type_ptr.hpp"
#include "Engine/Render/RenderManager.h"
#include "Engine/Material/MaterialPreview.h"

namespace NovaEngine {
	
	Material::Material(MaterialDef* def) {
		_techniques = def;
		setTechnique("Default");
		_previewGenerator = nullptr;
		_needsUpdatePreview = true;
	}

	Material::~Material() {
		delete _techniques;
	}

	void Material::setTechnique(std::string name) {
		_activeTechnique = getTechnique(name);
	}

	void Material::setTechnique(TechniqueDef* technique) {
		_activeTechnique = technique;
	}

	TechniqueDef* Material::getTechnique(std::string name) {
		return _techniques->getTechnique(name);
	}

	RenderState& Material::getRenderState(std::string technique) {
		return _activeTechnique->getRenderState();
	}

	void Material::setRenderState(RenderState& state, std::string technique) {
		_activeTechnique->setRenderState(state);
	}

	void Material::setUseMaterialColors(bool use) {

	}

	void Material::setLights(std::vector<Light*>& lights) {
		if(!_useLights) return;
		_directionalLightCache.clear();
		_pointLightCache.clear();
		for (Light* light : lights) {
			if (light->getType() == LightType::Ambient) {
				setVector4("AmbientLight", light->getColor().toVec4());
			} else if (light->getType() == LightType::Directional) {
				_directionalLightCache.push_back((DirectionalLight*)light);
			} else if (light->getType() == LightType::Point) {
				_pointLightCache.push_back((PointLight*)light);
			}
		}
		
		for (int i = 0; i < _directionalLightCache.size(); i++) {
			DirectionalLight* light = _directionalLightCache[i];
			setVector3("DirLights[" + std::to_string(i) + "].Position", light->getLocation());
			setVector3("DirLights[" + std::to_string(i) + "].Direction", light->getDirection());
			setVector3("DirLights[" + std::to_string(i) + "].Color", light->getColor().toVec3());
			setFloat("DirLights[" + std::to_string(i) + "].Intensity", light->getIntensity());
		}
		setInt("DirLightSize", _directionalLightCache.size());

		for (int i = 0; i < _pointLightCache.size(); i++) {
			PointLight* light = _pointLightCache[i];
			setVector3("PointLights[" + std::to_string(i) + "].Position", light->getPosition());
			setVector3("PointLights[" + std::to_string(i) + "].Color", light->getColor().toVec3());
			setFloat("PointLights[" + std::to_string(i) + "].Radius", light->getRadius());
			setBool("PointLights[" + std::to_string(i) + "].HasShadowMap", light->hasDepthMap());
			if (light->hasDepthMap()) {
				setTexture("PointLights[" + std::to_string(i) + "].ShadowMap", light->getDepthMap());
			}
		}
		setInt("PointLightCount", _pointLightCache.size());
	}

	bool Material::isShaderMatParamExist(std::string name) {
		return _activeTechnique->getShader()->getSUniform(name) != nullptr;
	}

	void Material::setInt(std::string name, int i) {
		setParam(name, VarType::Int, (void*)&i);
	}

	void Material::setFloat(std::string name, const float& f) {
		setParam(name, VarType::Float, (void*)&f);
	}

	void Material::setBool(std::string name, bool b) {
		setParam(name, VarType::Bool, (void*)&b);
	}

	void Material::setVector2(std::string name, glm::vec2 vec) {
		setParam(name, VarType::Vector2, (void*)&vec);
	}

	void Material::setVector3(std::string name, glm::vec3 vec) {
		setParam(name, VarType::Vector3, (void*)&vec);
	}

	void Material::setVector4(std::string name, glm::vec4 vec) {
		setParam(name, VarType::Vector4, (void*)&vec);
	}

	void Material::setMatrix3(std::string name, glm::mat3 mat) {
		setParam(name, VarType::Matrix3, (void*)&mat);
	}

	void Material::setMatrix4(std::string name, glm::mat4 mat) {
		setParam(name, VarType::Matrix4, (void*)&mat);
	}

	void Material::setTexture(std::string name, Texture* texture, bool visibleInEditor) {
		_activeTechnique->setParam<Texture*>(name, texture);
	}

	void Material::setParam(std::string name, const VarType& type, void* data) {
		if (type == VarType::Int) {
			_activeTechnique->setParam<int>(name, *((int*)data));
		} else if (type == VarType::Float) {
			_activeTechnique->setParam<float>(name, *((float*)data));
		} else if (type == VarType::Bool) {
			_activeTechnique->setParam<bool>(name, *((bool*)data));
		} else if (type == VarType::Vector2) {
			_activeTechnique->setParam<glm::vec2>(name, *((glm::vec2*)data));
		} else if (type == VarType::Vector3) {
			_activeTechnique->setParam<glm::vec3>(name, *((glm::vec3*)data));
		} else if (type == VarType::Vector4) {
			_activeTechnique->setParam<glm::vec4>(name, *((glm::vec4*)data));
		} else if (type == VarType::Matrix3) {
			_activeTechnique->setParam<glm::mat3>(name, *((glm::mat3*)data));
		} else if (type == VarType::Matrix4) {
			_activeTechnique->setParam<glm::mat4>(name, *((glm::mat4*)data));
		} else {
			std::cout << name << " -> wrong" << std::endl;
		}
	}

	void Material::setCamera(Camera * camera) {
		this->setMatrix4("ProjectionMatrix", camera->getProjectionMatrix());
		this->setMatrix4("ViewMatrix", camera->getViewMatrix());
	}

	void Material::render(Renderer* renderer, Mesh* mesh) {
		uploadParamsToGPU(renderer);
		renderer->renderMesh(mesh, 0);
		renderer->clearActiveTextures();
	}

	void Material::uploadParamsToGPU(Renderer* renderer) {
		if (_activeTechnique == nullptr) return;
		Shader* shader = _activeTechnique->getShader(renderer);
		if (shader == nullptr) return;
		renderer->setShader(shader);
		_activeTechnique->applyParams(renderer);
	}

	void Material::setUseLight(bool use) {
		_useLights = use;
	}

	void Material::setFilename(const std::string filename) {
		_filename = filename;
	}

	std::string Material::getFilename() const {
		if (_filename.length() == 0) {
			return getName();
		}
		return _filename;
	}

	bool& Material::isUsingLights() {
		return _useLights;
	}

	std::string Material::getName() const {
		return _techniques->getName();
	}

	File* Material::getFileSource() {
		return _fileSource;
	}

	void Material::setFileSource(const File& file) {
		_fileSource = new File(file);
	}

	void Material::updatePreview(RenderManager* renderManager) {
		if (_activeTechnique == nullptr) return;
		if (_activeTechnique->getShader() == nullptr) return;
		if (_activeTechnique->getShader()->getProgramCount() != 2) return;

		if (_previewGenerator == nullptr) {
			_previewGenerator = new MaterialPreview(renderManager, this);
		}
		if (_needsUpdatePreview) {
			_previewGenerator->render();
			_needsUpdatePreview = false;
		}
	}
	Texture* Material::getPreviewTexture() {
		if (_previewGenerator != nullptr) {
			return _previewGenerator->getOutput();
		}
		return nullptr;
	}
	void Material::setUpdatePreview() {
		_needsUpdatePreview = true;
	}

	void Material::recompile() {
		_activeTechnique->recompile();
		(_activeTechnique->getShader());
		//_activeTechnique->getShader()->compile();
	}
}