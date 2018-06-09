#include "Editor/Project.h"
#include "Editor/MetaManager.h"
#include "Engine/Render/Mesh/Shape.h"
#include "Engine/Utils/TangentGenerator.h"
#include "Engine/Terrain/TerrainNode.h"

namespace NovaEngine {
	
	Project::Project(Spatial* mainScene, ViewPort* viewPort) : _viewPort(viewPort) {
		_mainScene = mainScene;
	}

	void Project::setFileTree(FileTree* rootTree) {
		_rootTree = rootTree;
	}

	void Project::setTempMaterial(Material * mat) {
		_temporalMaterial = mat;
	}

	void Project::init(std::string path) {
		_path = path;
		if (!getFile().isExist()) {
			getFile().mkdirs();
		}
	}

	void Project::loadScene(Scene* scene) {
		SceneData sceneData = mapSceneData(scene);
		if(sceneData.Childs.size() == 0) return;
		/*_mainScene->getChilds()[0]->setLocation(sceneData.Childs[0].Location);
		_mainScene->getChilds()[0]->setRotation(sceneData.Childs[0].Rotation);
		sceneData.Childs.erase(sceneData.Childs.begin());*/

		_mainScene->getLights().clear();
		for (Light* light : sceneData.Lights) {
			_mainScene->addLight(light);
		}

		int i = 0;
		for (SceneData& child : sceneData.Childs) {
			initSpatial(child, _mainScene, i++);
		}
	}

	void Project::initSpatial(SceneData& data, Spatial* root, int index) {
		Spatial* child = nullptr;
		if (index < root->getChildCount()) {
			child = root->getChilds()[index];
		}
		bool usingSource = false;

		if (data.MeshSource.length() > 0) {
			FileData* meshData = _rootTree->findData(data.MeshSource);
			if (meshData != nullptr) {
				child = static_cast<Spatial*>(meshData->data)->clone();
				usingSource = true;
			}
		}
		if (child == nullptr && data.IsObj) {
			child = new GameObject(data.Name);
		} else if(child == nullptr) {
			child = new Spatial(data.Name);
		}

		child->setLocation(data.Location);
		child->setRotation(data.Rotation);
		child->setScale(data.Scale);
		child->setFileSource(File(data.MeshSource));

		std::cout << child->getName() << " - " << data.Name << std::endl;

		for (Light* light : data.Lights) {
			child->addLight(light);
		}

		if (data.IsObj) {
			GameObject* obj = static_cast<GameObject*>(child);
			FileData* matData = _rootTree->findData(data.MaterialSource);
			Material* mat = nullptr;
			if (matData != nullptr && matData->data != nullptr) {
				mat = static_cast<Material*>(matData->data);
				obj->setMaterial(mat);
			}

			int typeSep = data.MeshSource.find_last_of(':');
			if (typeSep > 0) {
				std::string init = data.MeshSource.substr(0, typeSep);
				std::string type = data.MeshSource.substr(typeSep + 1);
				if (init == "IMesh") {
					if (type == "Box") {
						obj->setMesh(new Box(glm::vec3(0, 0, 0), 0.5, 0.5, 0.5));
					} else if (type == "Quad") {
						obj->setMesh(new Quad(1, 1, false));
					} else if (type == "Plane") {
						obj->setMesh(new Quad(1, 1, false, true));
					} else if (type == "Sphere") {
						obj->setMesh(new Sphere(30, 30, 1, true, false));
					} else if (type == "TTClod") {
						delete child;
						child = new TerrainQuadTree(_viewPort, _assetManager,nullptr);
						child->setMaterial(mat);
					}
					/*try {
						if(obj->getMesh() != nullptr)
							TangentGenerator::generateTangentAndBitangents(obj->getMesh());
					} catch (std::exception& e) {}*/
				}
			}

			if (obj->getMaterial() == nullptr) {
				obj->setMaterial(_temporalMaterial);
			}
		}

		int i = 0;
		for (SceneData& childData : data.Childs) {
			initSpatial(childData, child, i++);
		}
		if (!root->isChildExist(child)) {
			root->addChild(child);
		}
	}

	void loadLights(jjson& json, SceneData& data) {
		if(json.find("Lights") == json.end()) return;
		for (jjson jLight : json["Lights"]) {
			Light* light = nullptr;
			std::string type = jLight["Type"].get<std::string>();
			glm::vec4 color = MetaManager::toVec4(jLight["Color"]);
			if (type == "AmbientLight") {
				light = new AmbientLight();
				light->setColor(makeRGBAf(color.x, color.y, color.z, color.w));
			} else if (type == "DirectionalLight") {
				DirectionalLight* dlight = new DirectionalLight();
				dlight->setColor(makeRGBAf(color.x, color.y, color.z, color.w));
				dlight->setDirection(MetaManager::toVec3(jLight["Direction"]));
				dlight->setPosition(MetaManager::toVec3(jLight["Position"]));
				glm::vec3 rot = MetaManager::toVec3(jLight["Rotation"]);
				dlight->setRotation(rot.x, rot.y, rot.z);
				light = dlight;
			} else if (type == "PointLight") {
				PointLight* dlight = new PointLight();
				dlight->setColor(makeRGBAf(color.x, color.y, color.z, color.w));
				dlight->setPosition(MetaManager::toVec3(jLight["Position"]));
				dlight->setRadius(jLight["Radius"].get<float>());
				light = dlight;
			}
			if (light != nullptr) {
				if (jLight.find("Intensity") != jLight.end()) {
					light->setIntensity(jLight["Intensity"].get<float>());
				}
				data.Lights.push_back(light);
			}
		}
	}

	void Project::loadSceneData(jjson& json, SceneData& data) {
		for (jjson jChild : json["Childs"]) {
			SceneData child;
			loadSceneData(jChild, child);
			data.Childs.push_back(child);
		}
		data.Name = json["Name"].get<std::string>();
		data.Location = MetaManager::toVec3(json["Location"]);
		data.Rotation = MetaManager::toVec3(json["Rotation"]);
		data.Scale = MetaManager::toVec3(json["Scale"]);
		data.IsObj = json["IsOBJ"].get<bool>();
		if (data.IsObj) {
			data.MaterialSource = json["Material"].get<std::string>();
		}

		if (json.find("MeshSource") != json.end()) {
			data.MeshSource = json["MeshSource"].get<std::string>();
		}

		loadLights(json, data);
	}

	SceneData Project::mapSceneData(Scene* scene) {
		SceneData sceneData;
		File file(getFile(), scene->getFilename());
		if (file.isExist()) {
			jjson data = Files::loadJson(file);
			for (jjson child : data) {
				loadSceneData(child, sceneData);
			}
		}
		return sceneData;
	}

	void saveLights(jjson& data, Spatial* spatial) {
		jjson jLights = jjson::array();
		for (Light* light : spatial->getLights()) {
			jjson jLight = jjson::object();
			jLight["Color"] = MetaManager::fromVec4(light->getColor().toVec4());
			jLight["Intensity"] = light->getIntensity();
			if (AmbientLight* dLight = dynamic_cast<AmbientLight*>(light)) {
				jLight["Type"] = "AmbientLight";
			} else if (DirectionalLight* dLight = dynamic_cast<DirectionalLight*>(light)) {
				jLight["Type"] = "DirectionalLight";
				jLight["Direction"] = MetaManager::fromVec3(dLight->getDirection());
				jLight["Position"] = MetaManager::fromVec3(dLight->getPosition());
				jLight["Rotation"] = MetaManager::fromVec3(dLight->getRotation());
			} else if (PointLight* dLight = dynamic_cast<PointLight*>(light)) {
				jLight["Type"] = "PointLight";
				jLight["Position"] = MetaManager::fromVec3(dLight->getPosition());
				jLight["Radius"] = dLight->getRadius();
			}
			jLights.push_back(jLight);
		}
		data["Lights"] = jLights;
	}

	void Project::saveScene(Scene* scene) {
		File file(getFile(), scene->getFilename());
		jjson data = jjson::array();
		IdProvider ids;
		saveSpatial(data, scene->getScene(), ids);
		Files::saveJson(data, file);
	}

	void Project::saveSpatial(jjson& json, Spatial* spatial, IdProvider& ids) {
		if(spatial == nullptr) return;
		if(spatial->isTemp()) return;
		jjson data = jjson::object();
		jjson jChilds = jjson::array();
		for (Spatial* child : spatial->getChilds()) {
			saveSpatial(jChilds, child, ids);
		}

		saveLights(data, spatial);

		data["Name"] = spatial->getName();
		data["Location"] = MetaManager::fromVec3(spatial->getLocation());
		data["Rotation"] = MetaManager::fromVec3(spatial->getRotation());
		data["Scale"] = MetaManager::fromVec3(spatial->getScale());

		if (GameObject* obj = dynamic_cast<GameObject*>(spatial)) {
			data["IsOBJ"] = true;
			data["Material"] = obj->getMaterial() == nullptr ? "" : (obj->getMaterial()->getFileSource() != nullptr ? obj->getMaterial()->getFileSource()->getPath() : "");
		} else {
			data["IsOBJ"] = false;
		}

		File* fileSource = spatial->getFileSource();
		if (fileSource != nullptr) {
			data["MeshSource"] = fileSource->getPath();
		} else {
			data["MeshSource"] = "null";
		}

		data["Childs"] = jChilds;
		json.push_back(data);
	}

	void Project::readFolderTree(File file, FileTree* baseTree, FileTree * tree, bool isAssetImport) {
		if (file.isDirectory()) {
			for (File subFile : file.listFiles()) {
				if (tree == baseTree && subFile.getName() == "cache") {
					continue;
				}
				FileTree* subTree = tree;
				if (subFile.isDirectory()) {
					subTree = tree->getOrCreateDir(subFile.getName());
				}
				readFolderTree(subFile, baseTree, subTree, isAssetImport);
			}
		} else {
			loadAsset(file, tree, isAssetImport);
		}
	}

	void Project::loadAsset(File file, FileTree* tree, bool isAssetImport) {
		//if (file.getPath()[file.getPath().length() - 1] == '/') return;
		FileType type = getFileType(file);
		if (type == FileType::Texture) {
			_assetManager->loadOrImportTexture(file, tree);
		} else if (type == FileType::Object3D) {
			_assetManager->loadOrImportModel(file, tree, _createdMaterials, isAssetImport);
			/*Spatial* model = _assetManager->loadModel(file.getPath());
			_mainScene->addChild(model);*/
		} else if (type == FileType::Material) {
			Material* mat = MetaManager::loadMaterialFromMeta(_assetManager, file, tree);
			if (mat != nullptr) {
				mat->setFileSource(file);
				FileData data = tree->addData(file, getFileType(file), mat);
				_createdMaterials.push_back(data);
			}
		} else {
			tree->addData(file, getFileType(file), new UnloadedAsset(file));
		}
	}

	void Project::load(AssetManager* assetManager) {
		_assetManager = assetManager;
		readFolderTree(getFile(), _rootTree, _rootTree, true);
		loadScene(new Scene(_mainScene, "TestScene.scene"));
	}

	void Project::save(bool saveEdited) {
		if (saveEdited) {
			for (FileData& file : _createdMaterials) {
				Material* mat = static_cast<Material*>(file.data);
				MetaManager::saveMaterialMeta(mat, *file.path);
			}
		}
		saveScene(new Scene(_mainScene, "TestScene.scene"));
	}

	void Project::reload(bool reimport) {
		readFolderTree(getFile(), _rootTree, _rootTree, reimport);
	}

	File Project::getFile() {
		return File(_path);
	}
	std::vector<FileData>& Project::getCreatedMaterials() {
		return _createdMaterials;
	}
}