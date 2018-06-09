#include "Engine/Material/Technique.h"
#include "Engine/Render/Shader.h"

namespace NovaEngine {
	/*Technique::Technique(std::string name) : _name(name), _shader(nullptr), _renderState() {
		_renderState.blendMode = BlendMode::Off;
		_renderState.depthTest = true;
		_renderState.faceCull = FaceCull::Back;
		_renderState.lineWidth = 1.0f;
		_renderState.wireframe = false;
	}

	Technique::~Technique() {
	}

	void Technique::setShader(Shader* shader) {
		_shader = shader;
	}

	Shader* Technique::getShader() {
		return _shader;
	}

	RenderState& Technique::getRenderState() {
		return _renderState;
	}
	void Technique::setRenderState(RenderState& state) {
		_renderState = state;
	}

	std::string Technique::getName() {
		return _name;
	}*/

	MaterialDef::MaterialDef(std::string name) {
		_name = name;
	}

	MaterialDef::~MaterialDef() {
		for (auto& i = _techniques.begin(); i != _techniques.end(); i++) {
			delete i->second;
		}
	}

	void MaterialDef::addTechnique(TechniqueDef* tech) {
		_techniques[tech->getName()] = tech;
	}

	TechniqueDef* MaterialDef::getTechnique(std::string name) {
		if (_techniques.find(name) == _techniques.end()) {
			return nullptr;
		}
		return _techniques[name];
	}
	std::string MaterialDef::getName() {
		return _name;
	}
	int MaterialDef::getCount() const {
		return (int)_techniques.size();
	}
}