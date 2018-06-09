#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H

#include <iostream>
#include <string>
#include <map>
#include "Engine/Scene/GameObject.h"
#include "Engine/Render/Mesh/Mesh.h"
#include "Engine/Render/Texture.h"
#include "Engine/Render/Texture2DAnimated.h"
#include "Engine/Render/Shader.h"

#include <assimp/Importer.hpp>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Editor/FileTree.h"

namespace NovaEngine {

	class Renderer;
	class IdProvider;
	class GOBJMapper;

	typedef std::map<std::string, Texture*> TextureCacheMap;
	typedef std::map<std::string, Shader*> ShaderCacheMap;

	class UnloadedAsset {
	public:
		File _file;
		inline UnloadedAsset(const File& file) : _file(file) { }
	};

	class AssetManager {
	public:
		AssetManager();
		~AssetManager();

		FileData* loadOrImportTexture(const File& file, FileTree* tree);

		// MESH LOADING
		Spatial* loadOrImportModel(const File& file, FileTree* tree, std::vector<FileData>& matList, bool isAssetImport);
		void processModelNode(aiNode* node, const aiScene* scene, Spatial* parent, const File& loadFileFolder, FileTree* masterTree, FileTree* textureTree, FileTree* materialsTree, bool import, IdProvider& ids, GOBJMapper* mapper, std::vector<FileData>& matList);
		void processModelTexture(aiMaterial* sourceMaterial, aiTextureType type, Material* mat, FileTree* tree, const File& loadFileFolder, std::map<int, aiTexture*>& eTextures);

		Spatial* loadModel(std::string path);
		void processNode(aiNode *node, const aiScene *scene, Spatial* parent, std::string directory);
		Mesh* processMesh(aiMesh *mesh, const aiScene *scene);
		void processAnimations(const aiScene* scene, Spatial* m);

		bool loadMaterialTextures(aiMaterial* material, aiTextureType type, Material* novaMat, std::string directory, std::map<int, aiTexture*> eTextures);
		// MATERIAL LOADING
		Material* loadMaterial(std::string name);
		// TEXTURE LOADING
		Texture* loadTexture(std::string path);
		void decodeFaces(unsigned char* faces, std::vector<void*>& faceList, int& w, int& h);
		unsigned char* unpackPixels(int startX, int startY, int w, int h, int ow, unsigned char* pixels);
		Texture* loadCubeMap(std::string path);
		Texture* loadGif(const File& file);
		
		Texture* loadHDR(const File& file);

		//Shader loading
		void recompileShader(Shader* shader);
		Shader* loadShader(std::string vertexShader, std::string tessControlShader, std::string tessEvalShader, std::string geomShader, std::string fragShader, std::string compShader);
		Shader* loadShader(std::string vertexShader, std::string geomShader, std::string fragShader);
		Shader* loadShader(std::string vertexShader, std::string fragShader);
		Shader* loadShaderFolder(std::string folder);

		Texture* loadRawHeightMap(const File& file, const FileType& type);

		TextureCacheMap& getLoadedTextures();

		void setRenderer(Renderer* renderer);
		Renderer* getRenderer();
	private:
		Renderer* _renderer;
		TextureCacheMap _textureCache;
		ShaderCacheMap _shaderCacheMap;

		Texture* loadTiff(const File& file);
		void loadTiff(const char *filename, unsigned char*& data, int &width, int &height);
	};
}

#endif // !ASSET_MANAGER_H