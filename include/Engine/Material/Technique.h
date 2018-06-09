#ifndef TECHNIQUE_H
#define TECHNIQUE_H

#include <string>
#include <map>
#include <memory>
#include <type_traits>

#include "Engine/Utils/Files.h"
#include "Engine/Render/Shader.h"
#include "Engine/Render/RenderState.h"
#include "Engine/Render/Renderer.h"
#include "glm/gtc/type_ptr.hpp"

namespace NovaEngine {

	enum class VarType {
		Int,
		Float,
		Vector2,
		Vector3,
		Vector4,
		Matrix3,
		Matrix4,
		Texture,
		TextureCubeMap,
		Texture3D,
		Bool
	};

	inline static VarType getVarTypeByString(const std::string& str) {
		if (str == "Int") {
			return VarType::Int;
		} else if (str == "Float") {
			return VarType::Float;
		} else if (str == "Vector2") {
			return VarType::Vector2;
		} else if (str == "Vector3") {
			return VarType::Vector3;
		} else if (str == "Vector4") {
			return VarType::Vector4;
		} else if (str == "Matrix3") {
			return VarType::Matrix3;
		} else if (str == "Matrix4") {
			return VarType::Matrix4;
		} else if (str == "Texture") {
			return VarType::Texture;
		} else if (str == "Texture2D") {
			return VarType::Texture;
		} else if (str == "TextureCubeMap") {
			return VarType::TextureCubeMap;
		} else if (str == "Texture3D") {
			return VarType::Texture3D;
		} else if (str == "Bool") {
			return VarType::Bool;
		}
		return VarType::Int;
	}

	typedef struct MaterialVarTexture {
		Texture* Texture;
		int ActiveUnit;
	} MaterialParamTexture;

	typedef struct MaterialParam {
		std::string Name;
		std::string Label;
		VarType Type;
		bool NeedsUpdate;
		bool IsNew;
		bool IsPreset;

		void* Data = nullptr;

		template<typename T> T* getDataPointer() {
			if (Data == nullptr) {
				Data = new T();
				IsNew = true;
				if (std::is_same<T, int>::value) { Type = VarType::Int; }
				if (std::is_same<T, float>::value) { Type = VarType::Float; }
				if (std::is_same<T, bool>::value) { Type = VarType::Bool; }
				if (std::is_same<T, glm::vec2>::value) { Type = VarType::Vector2; }
				if (std::is_same<T, glm::vec3>::value) { Type = VarType::Vector3; }
				if (std::is_same<T, glm::vec4>::value) { Type = VarType::Vector4; }
				if (std::is_same<T, glm::mat3>::value) { Type = VarType::Matrix3; }
				if (std::is_same<T, glm::mat4>::value) { Type = VarType::Matrix4; }
				if (std::is_same<T, MaterialVarTexture>::value) { Type = VarType::Texture; }
			}
			return (T*)Data;
		};

		template<typename T> void setData(T data) {
			if (!std::is_same<T, MaterialVarTexture>::value) {
				(*getDataPointer<T>()) = data;
			}
			this->NeedsUpdate = true;
		}

		template<> void setData<Texture*>(Texture* texture) {
			MaterialVarTexture* data = getDataPointer<MaterialVarTexture>();
			data->Texture = texture;
			this->NeedsUpdate = true;
		}

		template<typename T> T& getData() {
			return *getDataPointer<T>();
		}

	} MaterialParam;

	typedef struct ParamSubBlock {
		std::string Name;
		std::string Define;
		std::vector<MaterialParam*> Params;
	} ParamSubBlock;

	typedef struct ParamBlock {
		std::string Name;
		std::vector<ParamSubBlock*> SubBlocks;
	} ParamBlock;

	class TechniqueDef {
	private:
		std::string _name;
		std::map<ProgramType, File> _shaderFiles;
		std::map<ProgramType, std::vector<std::string>> _shaderSources;
		std::map<std::string, MaterialParam*> _params;
		std::vector<ParamBlock> _blockParams;
		ParamBlock _otherParamsBlock;
		ParamSubBlock* _otherParamSubBlock;
		std::map<std::string, bool> _defineSettings;
		bool _isComputeTechnique;
		bool _needsRecompile;
		bool _needsReUseParams;
		Shader* _activeShader;
		int _nextTextureUnit;

		RenderState _renderState;

		inline std::string applyDefines(std::vector<std::string> lines) {
			std::string source = "";
			std::string defines = "";
			for (auto& i = _defineSettings.begin(); i != _defineSettings.end(); i++) {
				if (i->second) {
					defines += "#define " + i->first + "\r\n";
				}
			}

			for (int i = 0; i < lines.size(); i++) {
				if (i == 1) {
					source += defines;
				}
				source += lines[i] + "\r\n";
			}

			return source;
		}
	public:
		inline ~TechniqueDef() {
			for (auto& i = _params.begin(); i != _params.end(); i++) {
				delete i->second->Data;
				delete i->second;
			}
		}

		inline TechniqueDef(const std::string& name) : _name(name), _shaderFiles(), _isComputeTechnique(false), _activeShader(nullptr), _nextTextureUnit(0), _renderState(), _needsRecompile(true), _needsReUseParams(false), _otherParamsBlock(), _otherParamSubBlock(nullptr) {
			_renderState.blendMode = BlendMode::Off;
			_renderState.depthTest = true;
			_renderState.faceCull = FaceCull::Back;
			_renderState.lineWidth = 1.0f;
			_renderState.wireframe = false;

			_otherParamsBlock.Name = "Other";

			_otherParamSubBlock = new ParamSubBlock();
			_otherParamSubBlock->Name = "Others";
			_otherParamSubBlock->Define = "";
			_otherParamsBlock.SubBlocks.push_back(_otherParamSubBlock);
		}

		inline void addShaderFile(const ProgramType& type, const File& file) {
			_shaderFiles[type] = file;
		}

		inline void loadShaders() {
			_shaderSources.clear();
			for (auto& i = _shaderFiles.begin(); i != _shaderFiles.end(); i++) {
				_shaderSources[i->first] = Files::readLines(i->second);
			}
		}

		inline void loadDefaultParams() {
			for (SUniform& su : _activeShader->getSUniforms()) {
				MaterialParam* param = nullptr;
				if (getParam(su.name, param)) continue;
				
				if (su.type == GL_SAMPLER_2D) {
					param = setParam<Texture*>(su.name, nullptr);
				} else if (su.type == GL_BOOL) {
					param = setParam<bool>(su.name, false);
				} else if (su.type == GL_FLOAT) {
					param = setParam<float>(su.name, su.floatVal);
				} else if (su.type == GL_INT) {
					param = setParam<int>(su.name, su.intVal);
				} else if (su.type == GL_FLOAT_VEC2) {
					param = setParam<glm::vec2>(su.name, su.vec2Val);
				} else if (su.type == GL_FLOAT_VEC3) {
					param = setParam<glm::vec3>(su.name, su.vec3Val);
				} else if (su.type == GL_FLOAT_VEC4) {
					param = setParam<glm::vec4>(su.name, su.vec4Val);
				}

				if (std::find(_otherParamSubBlock->Params.begin(), _otherParamSubBlock->Params.end(), param) == _otherParamSubBlock->Params.end()) {
					_otherParamSubBlock->Params.push_back(param);
				}
			}
		}

		inline Shader* getShader(Renderer* renderer) {
			if (_needsRecompile) {
				if (_activeShader != nullptr) {
					delete _activeShader;
					_activeShader = nullptr;
				}
				_activeShader = new Shader();
				for (auto& i = _shaderSources.begin(); i != _shaderSources.end(); i++) {
					_activeShader->addShader(i->first, applyDefines(i->second), "");
				}
				_activeShader->compile();
				loadDefaultParams();
				_needsRecompile = false;
				_needsReUseParams = true;
				if (renderer) {
					renderer->setInvalidateShader();
				}
			}
			return _activeShader;
		}

		inline void recompile() {
			loadShaders();
			_needsRecompile = true;
			_needsReUseParams = true;
		}

		inline Shader* getShader() {
			return getShader(nullptr);
		}

		inline std::vector<ParamBlock>& getParamBlocks() {
			return _blockParams;
		}

		inline ParamBlock& getOtherParamsBlock() {
			return _otherParamsBlock;
		}

		inline void addBlock(const ParamBlock& block) {
			_blockParams.push_back(block);
		}

		inline void setDefine(std::string name, bool def) {
			_defineSettings[name] = def;
			_needsRecompile = true;
		}

		inline bool& getDefineState(std::string name) {
			return _defineSettings[name];
		}

		inline std::map<std::string, bool>& getDefineSettings() {
			return _defineSettings;
		}

		inline void applyParams(Renderer* renderer) {
			if (_activeShader == nullptr) return;
			for (auto& i = _params.begin(); i != _params.end(); i++) {
				MaterialParam* param = i->second;
				if (param->Type == VarType::Texture || param->NeedsUpdate || _needsReUseParams) {
					param->NeedsUpdate = false;
					std::string& name = param->Name;
					switch (param->Type) {
					case VarType::Int:
						_activeShader->setUniform1i(name, param->getData<int>());
						break;
					case VarType::Bool:
						_activeShader->setUniform1i(name, param->getData<bool>() ? 1 : 0);
						break;
					case VarType::Float:
						_activeShader->setUniform1f(name, param->getData<float>());
						break;
					case VarType::Vector2:
						_activeShader->setUniform2fv(name, glm::value_ptr(param->getData<glm::vec2>()));
						break;
					case VarType::Vector3:
						_activeShader->setUniform3fv(name, glm::value_ptr(param->getData<glm::vec3>()));
						break;
					case VarType::Vector4:
						_activeShader->setUniform4fv(name, glm::value_ptr(param->getData<glm::vec4>()));
						break;
					case VarType::Matrix3:
						_activeShader->setUniformMatrix3fv(name, glm::value_ptr(param->getData<glm::mat3>()));
						break;
					case VarType::Matrix4:
						_activeShader->setUniformMatrix4fv(name, glm::value_ptr(param->getData<glm::mat4>()));
						break;
					case VarType::Texture:
						MaterialVarTexture& data = param->getData<MaterialVarTexture>();
						if (data.Texture != nullptr) {
							renderer->setTexture(data.Texture, data.ActiveUnit);
							_activeShader->setUniform1i(name, data.ActiveUnit);
						}
						break;
					}
				}
			}
			_needsReUseParams = false;
		}

		template<typename T>
		inline MaterialParam* setParam(std::string name, T data) {
			MaterialParam* param = nullptr;
			if (!getParam(name, param)) {
				param = new MaterialParam;
				param->Name = name;
				param->Label = name;
				param->IsPreset = false;
				_params[name] = param;
			}

			param->setData<T>(data);
			return param;
		}

		template<> inline MaterialParam* setParam<Texture*>(std::string name, Texture* texture) {
			MaterialParam* param = setParam<MaterialVarTexture>(name, MaterialVarTexture());
			MaterialVarTexture& data = param->getData<MaterialVarTexture>();
			if (param->IsNew) {
				data.ActiveUnit = _nextTextureUnit++;
			}
			data.Texture = texture;
			param->IsNew = false;
			return param;
		}

		inline Texture* getTextureFromParam(MaterialParam* param) {
			return param->getData<MaterialVarTexture>().Texture;
		}

		inline Texture* getTextureParam(std::string name) {
			MaterialParam* param = getParam(name);
			if (param == nullptr) return nullptr;
			MaterialVarTexture& data = param->getData<MaterialVarTexture>();
			return data.Texture;
		}

		inline bool getParam(std::string name, MaterialParam*& param_out) {
			if (_params.find(name) != _params.end()) {
				param_out = _params[name];
				return true;
			}
			return false;
		}
		
		inline MaterialParam* getParam(std::string name) {
			MaterialParam* param = nullptr;
			getParam(name, param);
			return param;
		}

		template<typename R>
		inline R getParamData(std::string name) {
			return getParam(name)->getData<R>();
		}

		template<> inline Texture* getParamData(std::string name) {
			MaterialParam* param = getParam(name);
			MaterialVarTexture& data = param->getData<MaterialVarTexture>();
			return data.Texture;
		}

		inline std::map<std::string, MaterialParam*>& getParams() {
			return _params;
		}

		inline RenderState& getRenderState() {
			return _renderState;
		}

		inline void setRenderState(const RenderState& state) {
			_renderState = state;
		}

		inline void setIsComputeShader(bool flag) {
			_isComputeTechnique = flag;
		}

		inline bool isComputeShader() const {
			return _isComputeTechnique;
		}

		inline std::string getName() {
			return _name;
		}
	};

	class MaterialDef {
	private:
		std::string _name;
		std::map<std::string, TechniqueDef*> _techniques;
	public:
		MaterialDef(std::string name);
		~MaterialDef();
		void addTechnique(TechniqueDef* tech);
		TechniqueDef* getTechnique(std::string name);
		std::string getName();
		int getCount() const;
	};
}

#endif // !TECHNIQUE_H