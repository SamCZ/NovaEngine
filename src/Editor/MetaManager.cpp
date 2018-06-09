#include "Editor/MetaManager.h"
#include "Engine/Utils/File.h"
#include "Engine/Render/Texture.h"
#include "Engine/Loaders/AssetManager.h"
#include "Editor/GOBJMapper.h"

namespace NovaEngine {
	jjson MetaManager::fromVec2(const glm::vec2& v2) {
		jjson obj = jjson::object();
		obj["x"] = v2.x;
		obj["y"] = v2.y;
		return obj;
	}
	jjson MetaManager::fromVec3(const glm::vec3& v3) {
		jjson obj = jjson::object();
		obj["x"] = v3.x;
		obj["y"] = v3.y;
		obj["z"] = v3.z;
		return obj;
	}
	jjson MetaManager::fromVec4(const glm::vec4& v4) {
		jjson obj = jjson::object();
		obj["x"] = v4.x;
		obj["y"] = v4.y;
		obj["z"] = v4.z;
		obj["w"] = v4.w;
		return obj;
	}

	glm::vec2 MetaManager::toVec2(jjson& json) {
		return glm::vec2(json["x"].get<float>(), json["y"].get<float>());
	}
	glm::vec3 MetaManager::toVec3(jjson& json) {
		return glm::vec3(json["x"].get<float>(), json["y"].get<float>(), json["z"].get<float>());
	}
	glm::vec4 MetaManager::toVec4(jjson& json) {
		return glm::vec4(json["x"].get<float>(), json["y"].get<float>(), json["z"].get<float>(), json["w"].get<float>());
	}

	void MetaManager::saveTextureMeta(Texture* tex, const File& file) {
		File metaFile(file.getPath() + ".meta");
		jjson json;
		json["AnisotropicFilter"] = tex->getAnisotropicFilter();
		json["IsNormalMap"] = tex->isNormalMap();
		json["MinFilter"] = (int)tex->minificationFilter;
		json["MagFilter"] = (int)tex->magnificationFilter;
		json["WrapS"] = (int)tex->getWrap(WrapAxis::S);
		json["WrapT"] = (int)tex->getWrap(WrapAxis::T);
		json["WrapR"] = (int)tex->getWrap(WrapAxis::R);
		Files::saveJson(json, metaFile);
	}

	void MetaManager::loadTextureMeta(Texture* tex, const File& file) {
		File metaFile(file.getPath() + ".meta");
		if (metaFile.isExist()) {
			jjson json = Files::loadJson(metaFile);
			tex->setAnisotropicFilter(json["AnisotropicFilter"].get<int>());
			tex->setNormalMap(json["IsNormalMap"].get<bool>());
			tex->setMinMagFilter((MinFilter)json["MinFilter"].get<int>(), (MagFilter)json["MagFilter"].get<int>());
			tex->setWrap(WrapAxis::S, (WrapMode)json["WrapS"].get<int>());
			tex->setWrap(WrapAxis::T, (WrapMode)json["WrapT"].get<int>());
			tex->setWrap(WrapAxis::R, (WrapMode)json["WrapR"].get<int>());
		}
	}
	void MetaManager::saveMaterialMeta(Material* material, const File& file) {
		jjson json;
		json["Source"] = material->getName();
		json["UseLights"] = material->isUsingLights();

		jjson params = jjson::object();
		for (const auto& i : material->getTechnique()->getParams()) {
			std::string name = i.first;
			MaterialParam* param = i.second;
			if (param->Type == VarType::Bool) {
				params["Bool:" + param->Name] = param->getData<bool>();
			} else if (param->Type == VarType::Float) {
				params["Float:" + param->Name] = param->getData<float>();
			} else if (param->Type == VarType::Int) {
				params["Int:" + param->Name] = param->getData<int>();
			} else if (param->Type == VarType::Texture) {
				if (param->getData<MaterialParamTexture>().Texture == nullptr) continue;
				params["Texture:" + param->Name] = param->getData<MaterialParamTexture>().Texture->getFile();
			} else if (param->Type == VarType::Vector2) {
				params["Vec2:" + param->Name] = fromVec2(param->getData<glm::vec2>());
			} else if (param->Type == VarType::Vector3) {
				params["Vec3:" + param->Name] = fromVec3(param->getData<glm::vec3>());
			} else if (param->Type == VarType::Vector4) {
				params["Vec4:" + param->Name] = fromVec4(param->getData<glm::vec4>());
			}
		}
		json["Params"] = params;

		jjson defines = jjson::object();
		for (const auto& i : material->getTechnique()->getDefineSettings()) {
			defines[i.first] = i.second;
		}
		json["Defines"] = defines;

		jjson jSTate = jjson::object();

		RenderState& state = material->getRenderState();
		jSTate["FaceCull"] = (int)state.faceCull;
		jSTate["DepthTest"] = state.depthTest;

		json["State"] = jSTate;
		Files::saveJson(json, file);
	}

	Material* MetaManager::loadMaterialFromMeta(AssetManager* assetManager, const File& file, FileTree* tree) {
		if (file.isExist()) {
			jjson json = Files::loadJson(file);
			if (json.find("Source") != json.end()) {
				Material* mat = assetManager->loadMaterial("Assets/Materials/" + json["Source"].get<std::string>());
				mat->setFilename(file.getName());
				mat->setUseLight(json["UseLights"].get<bool>());
				for (auto it = json["Defines"].begin(); it != json["Defines"].end(); it++) {
					std::string defineName = it.key();
					bool defineValue = it->get<bool>();
					mat->getTechnique()->setDefine(defineName, defineValue);
				}
				for (auto it = json["Params"].begin(); it != json["Params"].end(); it++) {
					std::string name = it.key();
					int dIndex = name.find_first_of(':');
					std::string type = name.substr(0, dIndex);
					name = name.substr(dIndex+1);

					if (type == "Bool") {
						mat->setBool(name, it->get<bool>());
					} else if (type == "Float") {
						mat->setFloat(name, it->get<float>());
					} else if (type == "Int") {
						mat->setInt(name, it->get<int>());
					} else if (type == "Texture") {
						FileData* texData = assetManager->loadOrImportTexture(it->get<std::string>(), tree);
						if (texData != nullptr) {
							Texture* tex = (Texture*)texData->data;
							if (tex != nullptr) {
								mat->setTexture(name, tex);
							}
						}
					} else if (type == "Vec2") {
						mat->setVector2(name, toVec2(*it));
					} else if (type == "Vec3") {
						mat->setVector3(name, toVec3(*it));
					} else if (type == "Vec4") {
						mat->setVector4(name, toVec4(*it));
					}
				}

				if (json.find("State") != json.end()) {
					jjson state = json["State"];
					mat->getRenderState().faceCull = (FaceCull)state["FaceCull"].get<int>();
					if (state.find("DepthTest") != state.end()) {
						mat->getRenderState().depthTest = state["DepthTest"].get<bool>();
					}
				}

				mat->recompile();
				return mat;
			}
		}
		return nullptr;
	}
	GOBJMapper* MetaManager::loadGOBJMap(const File& file) {
		GOBJMapper* map = new GOBJMapper();
		File mapFile(file.getParentFile(), file.getName() + ".map");
		if (mapFile.isExist()) {
			jjson data = Files::loadJson(mapFile);
			for (auto it = data.begin(); it != data.end(); it++) {
				int index = std::stoi(it.key());
				map->MaterialMap[index] = it->get<std::string>();
			}
		}
		return map;
	}

	void MetaManager::saveGOBJMap(GOBJMapper* mapper, const File& file) {
		File mapFile(file.getParentFile(), file.getName() + ".map");
		jjson data;
		for (std::map<int, std::string>::iterator i = mapper->MaterialMap.begin(); i != mapper->MaterialMap.end(); i++) {
			data[std::to_string(i->first)] = i->second;
		}
		Files::saveJson(data, mapFile);
	}
}