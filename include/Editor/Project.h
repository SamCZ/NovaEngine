#ifndef PROJECT_ZIDAN_H
#define PROJECT_ZIDAN_H

#include "Engine/Loaders/AssetManager.h"
#include "Engine/Scene/GameObject.h"
#include "Engine/Utils/File.h"
#include "Editor/Scene.h"
#include "Engine/Utils/File.h"
#include "Engine/Utils/Files.h"
#include "Engine/Utils/IdProvider.h"
#include "Editor/FileTree.h"

#include "Engine/Light/AmbientLight.h"
#include "Engine/Light/DirectionalLight.h"
#include "Engine/Light/PointLight.h"

namespace NovaEngine {
	class SceneData {
	public:
		std::string Name;
		std::vector<SceneData> Childs;
		glm::vec3 Location;
		glm::vec3 Rotation;
		glm::vec3 Scale;
		std::vector<Light*> Lights;
		bool IsObj;
		std::string MaterialSource;
		std::string MeshSource;
	};

	class Project {
	private:
		Spatial* _mainScene;
		ViewPort* _viewPort;
		FileTree* _rootTree;
		Material* _temporalMaterial;
		std::string _path;
		AssetManager* _assetManager;
		std::vector<FileData> _createdMaterials;

		void saveScene(Scene* scene);
		void loadScene(Scene* scene);

		void initSpatial(SceneData& data, Spatial* root, int index);
		void loadSceneData(jjson& json, SceneData& data);
		SceneData mapSceneData(Scene* scene);

		void saveSpatial(jjson& json, Spatial* spatial, IdProvider& ids);

		void readFolderTree(File file, FileTree* baseTree, FileTree* tree, bool isAssetImport);
	public:
		Project(Spatial* mainScene, ViewPort* viewPort);

		void setFileTree(FileTree* rootTree);
		void setTempMaterial(Material* mat);

		void init(std::string path);
		void load(AssetManager* assetManager);
		void save(bool saveEdited = false);

		void reload(bool reimport);
		void loadAsset(File file, FileTree* tree, bool isAssetImport);

		File getFile();

		std::vector<FileData>& getCreatedMaterials();
	};
}

#endif // !PROJECT_ZIDAN_H