#include "Engine/Loaders/AssetManager.h"

#include <map>
#include "Engine/Utils/Buffers.h"
#include "Engine/Render/Texture2D.h"
#include "Engine/Render/TextureCubeMap.h"
//#include "NanoVG/stb_image.h"
#include "STBI/stb_image.h"
#include "Engine/Utils/File.h"
#include "Engine/Utils/Files.h"
#include "Engine/Utils/Utils.h"
#include "Engine/Utils/TangentGenerator.h"
#include "Engine/Render/Renderer.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>

// Define TINY_DNG_LOADER_IMPLEMENTATION and STB_IMAGE_IMPLEMENTATION in only one *.cc
#define TINY_DNG_LOADER_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "NanoVG/tiny_dng_loader.h"
#include "Editor/MetaManager.h"
#include "Engine/Utils/IdProvider.h"
#include "Editor/GOBJMapper.h"

//#define STB_IMAGE_WRITE_IMPLEMENTATION
//#include "NanoVG/stb_image_write.h"
#include "STBI/stb_image_write.h"

#include "GifLib/gif_file.h"
#include "GifLib/gif_list.h"

#include "Engine/Utils/ColorBitmap.h"

#include "TinyTIFF/tinytiffreader.h"

#include "IL/il.h"
#include "IL/ilu.h"
#include "IL/ilut.h"

namespace NovaEngine {

	glm::vec3 aiVecToGlm(const aiColor3D& col) {
		return glm::vec3(col.r, col.g, col.b);
	}

	AssetManager::AssetManager() {
		_renderer = nullptr;
		ilInit();
		iluInit();
		ilutInit();
		ilutRenderer(ILUT_OPENGL);

		ILenum ilError = ilGetError();
		if (ilError != IL_NO_ERROR) {
			printf("Error initializing DevIL! %s\n", iluErrorString(ilError));
		}
	}

	AssetManager::~AssetManager() {
		for (std::map<std::string, Texture*>::iterator i = _textureCache.begin(); i != _textureCache.end(); i++) {
			delete i->second;
		}
		_textureCache.clear();
	}

	FileData* AssetManager::loadOrImportTexture(const File& file, FileTree* tree) {
		File assetFile(tree->getPath(), File(file).getName());
		if (file != assetFile && !assetFile.isExist()) {
			Files::copyFile(File(file), assetFile);
		}
		std::string e = file.getExtension();
		Texture* tex = nullptr;
		if (e == "r16") {
			tex = loadRawHeightMap(file, FileType::Raw16);
		} else if(e == "tiff" || e == "tif") {
			tex = loadTiff(file);
		} else {
			tex = loadTexture(assetFile.getPath());
		}
		if (tex == nullptr) {
			return nullptr;
		}
		return new FileData(tree->addData(assetFile, FileType::Texture, tex));
	}

	Spatial* AssetManager::loadOrImportModel(const File& file, FileTree* tree, std::vector<FileData>& matList, bool isAssetImport) {
		if (!isAssetImport) {
			File* folderName = nullptr;
			std::string modelName = file.getCleanName();
			int tryCount = 0;
			do {
				if (folderName != nullptr) delete folderName;
				folderName = new File(tree->getPath(), tryCount == 0 ? modelName : (modelName + "_" + std::to_string(tryCount)));
				tryCount++;
			} while (folderName->isExist());

			if (!folderName->isExist()) {
				folderName->mkdirs();
			}
			tree = tree->getOrCreateDir(modelName);
		}
		File assetFile(tree->getPath(), File(file).getName());
		if (file != assetFile && !assetFile.isExist()) {
			Files::copyFile(File(file), assetFile);
		}

		bool import = file != assetFile;

		if (import) {
			for (File folderFiles : file.getParentFile().listFiles()) {
				if (folderFiles.getExtension(true) == "mtl") {
					Files::copyFile(folderFiles, File(assetFile.getParentFile(), folderFiles.getName()));
				}
			}
		}

		Assimp::Importer importer;
		/*importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_READ_TEXTURES, true);
		importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_SEARCH_EMBEDDED_TEXTURES, true);
		importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_READ_ALL_MATERIALS, true);*/
		const aiScene *scene = importer.ReadFile(assetFile.getPath(), aiProcessPreset_TargetRealtime_Quality |
												 aiProcess_FindInstances |
												 aiProcess_ValidateDataStructure |
												 aiProcess_OptimizeMeshes | aiProcess_FlipUVs | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace); //aiProcess_PreTransformVertices
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
			std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
			tree->addData(assetFile, FileType::Unsupported, new UnloadedAsset(assetFile));
			return nullptr;
		}
		//import = true;
		GOBJMapper* mapper = import ? new GOBJMapper() : MetaManager::loadGOBJMap(assetFile);

		FileTree* textureSubTree = tree->getOrCreateDir("Textures");
		FileTree* materialSubTree = tree->getOrCreateDir("Materials");
		File textureDir = textureSubTree->getPath();
		if (!textureDir.isExist() && import) {
			textureDir.mkdirs();
		}
		File materialsDir = materialSubTree->getPath();
		if (!materialsDir.isExist() && import) {
			materialsDir.mkdirs();
		}

		Spatial* node = new Spatial(assetFile.getName());
		node->setFileSource(assetFile);
		
		processAnimations(scene, node);
		try {
			IdProvider ids;
			processModelNode(scene->mRootNode, scene, node, file.getParentFile(), tree, textureSubTree, materialSubTree, import, ids, mapper, matList);
		} catch (std::exception& e) {
			return nullptr;
		}
		if (scene->HasAnimations())
			node->getAnimator()->animations[node->getAnimator()->currentAnim].buildBoneTree(scene, scene->mRootNode, &node->getAnimator()->animations[node->getAnimator()->currentAnim].root, node);

		if(import) MetaManager::saveGOBJMap(mapper, assetFile);
		delete mapper;

		/*if (assetFile.getName() == "cube.obj") {
			GameObject* gg = (GameObject*)node->getChilds()[0];
			Mesh* mesh = gg->getMesh();

			IntBuffer* in = (IntBuffer*)mesh->getVertexBuffer(MeshBuffer::Type::Index)->getBuffer();
			FloatBuffer* pb = (FloatBuffer*)mesh->getVertexBuffer(MeshBuffer::Type::Position)->getBuffer();
			FloatBuffer* nb = (FloatBuffer*)mesh->getVertexBuffer(MeshBuffer::Type::Normal)->getBuffer();
			FloatBuffer* tb = (FloatBuffer*)mesh->getVertexBuffer(MeshBuffer::Type::TexCoord)->getBuffer();

			std::cout << "indices" << std::endl;
			in->print(3);

			std::cout << "positions" << std::endl;
			pb->print(3);

			std::cout << "normals" << std::endl;
			nb->print(3);

			std::cout << "texdata" << std::endl;
			tb->print(2);
		}*/

		importer.FreeScene();
		tree->addData(assetFile, FileType::Object3D, node);
		std::cout << "Model was loaded. " << assetFile << std::endl;
		return node;
	}

	void AssetManager::processModelNode(aiNode* node, const aiScene* scene, Spatial* parent, const File& loadFileFolder, FileTree* masterTree, FileTree* textureTree, FileTree* materialsTree, bool import, IdProvider& ids, GOBJMapper* mapper, std::vector<FileData>& matList) {
		std::map<int, aiTexture*> eTextures;
		for (int i = 0; i < scene->mNumTextures; i++) {
			eTextures[i] = scene->mTextures[i];
		}

		aiVector3D position;
		aiVector3D rotation;
		aiVector3D scale;

		aiMatrix4x4 transform = node->mTransformation;
		transform.Decompose(scale, rotation, position);

		for (unsigned int i = 0; i < node->mNumMeshes; i++) {
			aiMesh* sourceMesh = scene->mMeshes[node->mMeshes[i]];
			Mesh* mesh = processMesh(sourceMesh, scene);
			if (mesh->getMode() != Mode::Triangles) {
				delete mesh;
				continue;
			}
			std::string name = std::string(node->mName.C_Str());
			GameObject* obj = new GameObject(std::string(node->mName.C_Str()));
			obj->setLocation(position.x, position.y, position.z);
			obj->setRotation(glm::degrees(rotation.x), glm::degrees(rotation.y), glm::degrees(rotation.z));
			obj->setScale(scale.x, scale.y, scale.z);
			obj->setMesh(mesh);
			/*try {
				TangentGenerator::generate(mesh, true, false);
			} catch (std::exception& e) { }*/
			int id = ids.getId();

			if (sourceMesh->mMaterialIndex >= 0) {
				aiMaterial* souceMaterial = scene->mMaterials[sourceMesh->mMaterialIndex];

				if (import) {
					File* materialFile = nullptr;
					int tryIndex = 0;
					do {
						if (materialFile != nullptr) delete materialFile;
						materialFile = new File(materialsTree->getPath(), tryIndex == 0 ? (name + ".mat") : (name + "_" + std::to_string(tryIndex) + ".mat"));
						tryIndex++;
					} while (materialFile->isExist());
					
					Material* mat = loadMaterial("Assets/Materials/PBR_IBL");
					mat->setFilename(materialFile->getName());
					mat->setFileSource(*materialFile);
					mat->setUseLight(true);
					obj->setMaterial(mat);

					{// Load material params
						unsigned int max = 1;
						int two_sided;
						if ((AI_SUCCESS == aiGetMaterialIntegerArray(souceMaterial, AI_MATKEY_TWOSIDED, &two_sided, &max)) && two_sided) {
							mat->getRenderState().faceCull = FaceCull::Off;
						}
						mat->getRenderState().faceCull = FaceCull::Off;

						aiColor3D diffuseColor(1, 1, 1);
						if (souceMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor) == AI_SUCCESS) {
							mat->setVector3("albedoColor", aiVecToGlm(diffuseColor));
						}

						aiColor3D specularColor(1, 1, 1);
						if (souceMaterial->Get(AI_MATKEY_COLOR_SPECULAR, specularColor) == AI_SUCCESS) {
							mat->setVector3("Specular", aiVecToGlm(specularColor));
						}

						aiColor3D ambientColor(1, 1, 1);
						if (souceMaterial->Get(AI_MATKEY_COLOR_AMBIENT, ambientColor) == AI_SUCCESS) {
							mat->setFloat("aoValue", ambientColor.r);
						}

						float specularPower = 0.0f;
						unsigned int maxSpec;
						if (aiGetMaterialFloatArray(souceMaterial, AI_MATKEY_SHININESS, &specularPower, &maxSpec) == AI_SUCCESS) {
							mat->setFloat("Shininess", specularPower);
						}

						float opacity = 1.0f;
						if (aiGetMaterialFloat(souceMaterial, AI_MATKEY_OPACITY, &opacity)) {
							std::cout << "Opacity" << opacity << std::endl;
						}
						processModelTexture(souceMaterial, aiTextureType::aiTextureType_DIFFUSE, mat, textureTree, loadFileFolder, eTextures);
						processModelTexture(souceMaterial, aiTextureType::aiTextureType_AMBIENT, mat, textureTree, loadFileFolder, eTextures);
						processModelTexture(souceMaterial, aiTextureType::aiTextureType_SPECULAR, mat, textureTree, loadFileFolder, eTextures);
						processModelTexture(souceMaterial, aiTextureType::aiTextureType_NORMALS, mat, textureTree, loadFileFolder, eTextures);
						processModelTexture(souceMaterial, aiTextureType::aiTextureType_SHININESS, mat, textureTree, loadFileFolder, eTextures);
						processModelTexture(souceMaterial, aiTextureType::aiTextureType_HEIGHT, mat, textureTree, loadFileFolder, eTextures);
						processModelTexture(souceMaterial, aiTextureType::aiTextureType_DISPLACEMENT, mat, textureTree, loadFileFolder, eTextures);
						processModelTexture(souceMaterial, aiTextureType::aiTextureType_OPACITY, mat, textureTree, loadFileFolder, eTextures);
					}

					MetaManager::saveMaterialMeta(mat, *materialFile);
					mapper->MaterialMap[id] = materialFile->getName();
					matList.push_back(materialsTree->addData(*materialFile, FileType::Material, mat));
					delete materialFile;
				} else {
					if (mapper->MaterialMap.find(id) != mapper->MaterialMap.end()) {
						std::string fileSource = mapper->MaterialMap[id];
						File matFile(materialsTree->getPath(), fileSource);
						if (materialsTree->dataExist(matFile)) {
							obj->setMaterial(static_cast<Material*>(materialsTree->getData(matFile).data));
						} else {
							Material* mat = MetaManager::loadMaterialFromMeta(this, matFile, masterTree);
							if (mat != nullptr) {
								mat->setFileSource(matFile);
								FileData data = materialsTree->addData(matFile, getFileType(matFile), mat);
								matList.push_back(data);
								obj->setMaterial(mat);
							}
						}
					}
				}
			}

			parent->addChild(obj);
		}

		for (unsigned int i = 0; i < node->mNumChildren; i++) {
			processModelNode(node->mChildren[i], scene, parent, loadFileFolder, masterTree, textureTree, materialsTree, import, ids, mapper, matList);
		}
	}

	void AssetManager::processModelTexture(aiMaterial* sourceMaterial, aiTextureType type, Material* mat, FileTree* tree, const File& loadFileFolder, std::map<int, aiTexture*>& eTextures) {
		for (unsigned int i = 0; i < sourceMaterial->GetTextureCount(type); i++) {
			aiString sourceTexName;
			sourceMaterial->GetTexture(type, i, &sourceTexName);
			std::string name = sourceTexName.C_Str();

			Texture* texture = nullptr;
			if (name[0] == '*') {
				int texIndex = std::stoi(name.substr(1));
				aiTexture* aiTex = eTextures[texIndex];
				unsigned char* aiData = (unsigned char*)aiTex->pcData;
				int colorFormat = 4;
				std::string type = aiTex->achFormatHint;
				std::cout << type << std::endl;
				if (type == "tga") {
					colorFormat = 3;
				} else if (type == "tif") {
					colorFormat = 4;
				}
				int w, h, n;
				unsigned char* data = stbi_load_from_memory(aiData, aiTex->mWidth, &w, &h, &n, colorFormat);
				if (data) {
					File* writeTexPath = nullptr;
					int tryIndex = 0;
					do {
						if (writeTexPath != nullptr) delete writeTexPath;
						writeTexPath = new File(tree->getPath(), tryIndex == 0 ? ("Texture.png") : ("Texture_" + std::to_string(tryIndex) + ".png"));
						tryIndex++;
					} while (writeTexPath->isExist());
					stbi_write_png(writeTexPath->getPath().c_str(), w, h, n, data, w * n);
					stbi_image_free(data);
					FileData* fData = loadOrImportTexture(*writeTexPath, tree);
					if (fData != nullptr && fData->data != nullptr) {
						texture = static_cast<Texture*>(fData->data);
					}
				}
			} else {
				File loadTexPath(loadFileFolder, name);
				if (!loadTexPath.isExist()) {
					loadTexPath = File(loadFileFolder, File(name).getCleanName() + ".tga");
					if (!loadTexPath.isExist()) {
						loadTexPath = File(loadFileFolder, name);
					}
				}

				if (!loadTexPath.isExist()) {
					loadTexPath = File(File(loadFileFolder, "Textures"), File(name).getName());
					if (!loadTexPath.isExist()) {
						if (!loadTexPath.isExist()) {
							loadTexPath = File(File(loadFileFolder, "Textures"), File(name).getCleanName() + ".tga");
							if (!loadTexPath.isExist()) {
								loadTexPath = File(File(loadFileFolder, "Textures"), File(name).getName());
							}
						}
					}
				}
				if (loadTexPath.isExist()) {
					FileData* data = loadOrImportTexture(loadTexPath, tree);
					texture = static_cast<Texture*>(data->data);
					delete data;
				}
			}
			if (texture != nullptr) {
				if (texture != nullptr) {
					texture->setWrap(WrapMode::Repeat);
					texture->setMinMagFilter(MinFilter::Trilinear, MagFilter::Bilinear);
					if (type == aiTextureType::aiTextureType_AMBIENT) {
						mat->setTexture("aoMap", texture);
					} else if (type == aiTextureType::aiTextureType_DIFFUSE) {
						mat->setTexture("albedoMap", texture);
					} else if (type == aiTextureType::aiTextureType_SPECULAR) {
						/*novaMat->setSpecularMap(texture);
						novaMat->getRenderState().blendMode = BlendMode::Alpha;
						novaMat->setBool("UseAlpha", true);
						novaMat->getRenderState().faceCull = FaceCull::Off;*/
					} else if (type == aiTextureType::aiTextureType_NORMALS) {
						mat->setTexture("normalMap", texture);
					} else if (type == aiTextureType::aiTextureType_SHININESS) {
						mat->setTexture("ShininessMap", texture);
					} else if (type == aiTextureType::aiTextureType_HEIGHT) {
						mat->setTexture("HeightMap", texture);
					} else if (type == aiTextureType::aiTextureType_DISPLACEMENT) {
						mat->setTexture("DisplacementMap", texture);
					} else if (type == aiTextureType::aiTextureType_OPACITY) {
						//novaMat->getRenderState().blendMode = BlendMode::Alpha;
						//texture->setWrap(WrapMode::ClampToEdge);
						mat->setTexture("AlphaMap", texture);
						mat->setBool("UseAlphaMap", true);
						//novaMat->alpha = true;
						//novaMat->getRenderState().faceCull = FaceCull::Off;
					}
				}
			}
			break;
		}
	}

	Spatial* AssetManager::loadModel(std::string path) {
		std::cout << "Loading model " << path << std::endl;
		Assimp::Importer importer;
		/*importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_READ_TEXTURES, true);
		importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_SEARCH_EMBEDDED_TEXTURES, true);
		importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_READ_ALL_MATERIALS, true);*/

		const aiScene *scene = importer.ReadFile(path, aiProcessPreset_TargetRealtime_Quality |
												 aiProcess_FindInstances |
												 aiProcess_ValidateDataStructure |
												 aiProcess_OptimizeMeshes | aiProcess_FlipUVs); //aiProcess_PreTransformVertices
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
			std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
			return nullptr;
		}
		std::string directory = File(path).getParent();
		Spatial* node = new Spatial(File(path).getName());
		processAnimations(scene, node);
		processNode(scene->mRootNode, scene, node, directory);
		
		if (scene->HasAnimations())
			node->getAnimator()->animations[node->getAnimator()->currentAnim].buildBoneTree(scene, scene->mRootNode, &node->getAnimator()->animations[node->getAnimator()->currentAnim].root, node);
		importer.FreeScene();
		return node;
	}

	void AssetManager::processNode(aiNode* node, const aiScene* scene, Spatial* parent, std::string directory) {
		// process all the node's meshes (if any)

		//std::cout << "Embedded textures: " << scene->mNumTextures << std::endl;
		std::map<int, aiTexture*> eTextures;
		for (int i = 0; i < scene->mNumTextures; i++) {
			eTextures[i] = scene->mTextures[i];
		}
		
		for (unsigned int i = 0; i < node->mNumMeshes; i++) {
			GameObject* obj = new GameObject(std::string(node->mName.C_Str()));
			//scene->mTextures[0]->pcData
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			Mesh* buidedMesh = processMesh(mesh, scene);
			if (buidedMesh->getMode() != Mode::Triangles) {
				/*if(buidedMesh->getMode() != Mode::Lines)
					continue;*/
				continue;
			}
			Material* novaMat = loadMaterial("Assets/Materials/Lighting");
			novaMat->setUseLight(true);
			if (mesh->mMaterialIndex >= 0) {
				aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

				//scene->mTextures[0]

				unsigned int max = 1;
				int two_sided;
				if ((AI_SUCCESS == aiGetMaterialIntegerArray(material, AI_MATKEY_TWOSIDED, &two_sided, &max)) && two_sided) {
					novaMat->getRenderState().faceCull = FaceCull::Off;
				}
				novaMat->getRenderState().faceCull = FaceCull::Off;


				/*aiString name;
				material->Get(AI_MATKEY_NAME, name);
				std::codut << name.C_Str() << std::endl;*/

				aiColor3D diffuseColor(1, 1, 1);
				if (material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor) == AI_SUCCESS) {
					novaMat->setVector3("Diffuse", aiVecToGlm(diffuseColor));
				}

				aiColor3D specularColor(1, 1, 1);
				if (material->Get(AI_MATKEY_COLOR_SPECULAR, specularColor) == AI_SUCCESS) {
					novaMat->setVector3("Specular", aiVecToGlm(specularColor));
				}

				aiColor3D ambientColor(1, 1, 1);
				if (material->Get(AI_MATKEY_COLOR_AMBIENT, ambientColor) == AI_SUCCESS) {
					novaMat->setVector3("Ambient", aiVecToGlm(ambientColor));
				}

				float specularPower = 0.0f;
				unsigned int maxSpec;
				if (aiGetMaterialFloatArray(material, AI_MATKEY_SHININESS, &specularPower, &maxSpec) == AI_SUCCESS) {
					novaMat->setFloat("Shininess", specularPower);
				}

				float opacity = 1.0f;
				if (aiGetMaterialFloat(material, AI_MATKEY_OPACITY, &opacity)) {
					std::cout << "Opacity" << opacity << std::endl;
				}

				loadMaterialTextures(material, aiTextureType::aiTextureType_DIFFUSE, novaMat, directory, eTextures);
				loadMaterialTextures(material, aiTextureType::aiTextureType_AMBIENT, novaMat, directory, eTextures);
				if (loadMaterialTextures(material, aiTextureType::aiTextureType_SPECULAR, novaMat, directory, eTextures)) {
					//obj->setRenderOrder(RenderOrder::Trasparent);
				}
				loadMaterialTextures(material, aiTextureType::aiTextureType_NORMALS, novaMat, directory, eTextures);
				loadMaterialTextures(material, aiTextureType::aiTextureType_SHININESS, novaMat, directory, eTextures);
				loadMaterialTextures(material, aiTextureType::aiTextureType_HEIGHT, novaMat, directory, eTextures);
				loadMaterialTextures(material, aiTextureType::aiTextureType_DISPLACEMENT, novaMat, directory, eTextures);
				if (loadMaterialTextures(material, aiTextureType::aiTextureType_OPACITY, novaMat, directory, eTextures)) {
					//obj->setRenderOrder(RenderOrder::Trasparent);
				}
			}
			buidedMesh->updateBounds();
			//buidedMesh->createCollisionData();
			//TangentGenerator::generateTangentAndBitangents(buidedMesh);
			try {
				//TangentGenerator::generate(buidedMesh, true, false);
			} catch (std::exception& e) {

			}
			obj->setMesh(buidedMesh);
			obj->setMaterial(novaMat);

			parent->addChild(obj);
		}
		// then do the same for each of its children

		for (unsigned int i = 0; i < node->mNumChildren; i++) {
			processNode(node->mChildren[i], scene, parent, directory);
		}
	}

	bool AssetManager::loadMaterialTextures(aiMaterial* material, aiTextureType type, Material* novaMat, std::string directory, std::map<int, aiTexture*> eTextures) {
		for (unsigned int i = 0; i < material->GetTextureCount(type); i++) {
			aiString str;
			material->GetTexture(type, i, &str);
			std::string stdStr = str.C_Str();
			File file(std::string(str.C_Str()));
			std::string texturePath = directory + std::string("/") + file.getPath();
			std::cout << "FBX: " << stdStr << std::endl;
			Texture* texture = nullptr;
			if (stdStr[0] == '*') {
				int texIndex = std::stoi(stdStr.substr(1));
				aiTexture* aiTex = eTextures[texIndex];

				if (_textureCache.find(texturePath) != _textureCache.end()) {
					texture =  static_cast<Texture2D*>(_textureCache[texturePath]);
				} else {
					unsigned char* aiData = (unsigned char*)aiTex->pcData;
					if (aiTex->mHeight == 0) {
						ImageFormat format = ImageFormat::RGBA8;
						int colorFormat = 4;
						std::string type = aiTex->achFormatHint;
						if (type == "tga") {
							colorFormat = 3;
							format = ImageFormat::RGB8;
						}
						if (type == "tif") {
							colorFormat = 4;
							format = ImageFormat::BGRA8;
						}
						int w, h, n;
						unsigned char* data = stbi_load_from_memory(aiData, aiTex->mWidth, &w, &h, &n, colorFormat);
						if (!data) {
							std::cout << "dataEmpty" << std::endl;
						}
						texture = new Texture2D(w, h, format, data);
					} else {
						unsigned char* data = new unsigned char[aiTex->mWidth * aiTex->mHeight * 4];
						memcpy(data, aiData, aiTex->mWidth * aiTex->mHeight * 4);
						texture = new Texture2D((int)aiTex->mWidth, (int)aiTex->mHeight, ImageFormat::RGBA8, aiData);
					}
					_textureCache[texturePath] = texture;
				}
			} else {
				if (file.getParentFile().getName() == "CZ805") {
					texturePath = File(directory).getParentFile().getPath() + std::string("/Textures/") + file.getName().substr(0, file.getName().find_last_of('.')) + ".tga";
				}
				if (!File(texturePath).isExist()) {
					//texturePath = directory + std::string("/") + file.getPath() + ".tga";
				}
				if (file.getName() == "d4ef9c5290adddbd86c486175755fe6e.jpg") {
					novaMat->setBool("Yaaa", true);
				}
				//std::cout << "Loading " << str.C_Str() << std::endl;
				texture = loadTexture(texturePath);
			}

			if (texture != nullptr) {
				if (texture != nullptr) {
					texture->setWrap(WrapMode::Repeat);
					texture->setMinMagFilter(MinFilter::Trilinear, MagFilter::Bilinear);
					if (type == aiTextureType::aiTextureType_AMBIENT) {
						novaMat->setTexture("AmbientMap", texture);
					} else if (type == aiTextureType::aiTextureType_DIFFUSE) {
						novaMat->setTexture("Diffuse", texture);
					} else if (type == aiTextureType::aiTextureType_SPECULAR) {
						/*novaMat->setSpecularMap(texture);
						novaMat->getRenderState().blendMode = BlendMode::Alpha;
						novaMat->setBool("UseAlpha", true);
						novaMat->getRenderState().faceCull = FaceCull::Off;*/
					} else if (type == aiTextureType::aiTextureType_NORMALS) {
						novaMat->setTexture("NormalMap", texture);
					} else if (type == aiTextureType::aiTextureType_SHININESS) {
						novaMat->setTexture("ShininessMap", texture);
					} else if (type == aiTextureType::aiTextureType_HEIGHT) {
						novaMat->setTexture("HeightMap", texture);
					} else if (type == aiTextureType::aiTextureType_DISPLACEMENT) {
						novaMat->setTexture("DisplacementMap", texture);
					} else if (type == aiTextureType::aiTextureType_OPACITY) {
						//novaMat->getRenderState().blendMode = BlendMode::Alpha;
						//texture->setWrap(WrapMode::ClampToEdge);
						novaMat->setTexture("AlphaMap", texture);
						novaMat->setBool("UseAlphaMap", true);
						//novaMat->alpha = true;
						//novaMat->getRenderState().faceCull = FaceCull::Off;
					}
				}
			}
			break;
		}
		return false;
	}

	Material* AssetManager::loadMaterial(std::string materialPath) {
		try {
			File file(materialPath);

			if (!file.isExist()) {
				throw std::runtime_error("Material folder \"" + materialPath + "\" is not exist !");
			}

			File configFile(file, "config.json");
			if (configFile.isExist()) {
				jjson jConfig = Files::loadJson(configFile);

				MaterialDef* materialDef = new MaterialDef(file.getName());

				for (jjson jTechnique : jConfig) {
					std::string materialName = jTechnique["Name"];
					std::string techniqueName = jTechnique["TechniqueName"];

					TechniqueDef* def = new TechniqueDef(techniqueName);

					bool isComputeShader = false;
					bool isFragmentShader = false;
					bool isVertexShader = false;
					File techniqueFolder(file, techniqueName);
					for (ProgramType shaderType : Shader::getAllProgramTypes()) {
						File shaderFile(techniqueFolder, "shader." + Shader::getShaderTypeExtensionName(shaderType));
						if (shaderFile.isExist()) {
							isComputeShader = shaderType == ProgramType::Compute;
							isFragmentShader = shaderType == ProgramType::Fragment;
							isVertexShader = shaderType == ProgramType::Vertex;

							def->addShaderFile(shaderType, shaderFile);
						}
					}
					def->setIsComputeShader(isComputeShader);

					if ((isFragmentShader || isVertexShader) && isComputeShader) {
						throw std::runtime_error("Material \"" + materialPath + "\" cannot be normal and compute shader at once !");
					}

					if ((isFragmentShader && isVertexShader && !isComputeShader) == false) {
						//throw std::runtime_error("Material \"" + name + "\": Normal shader must have vertex and fragment program !");
					}

					if (jTechnique.find("UniformBlocks") != jTechnique.end()) {
						for (jjson jUniformBlocks : jTechnique["UniformBlocks"]) {
							std::string blockName = jUniformBlocks["BlockName"];
							
							ParamBlock block;
							block.Name = blockName;

							for (jjson jUniformSubBlocks : jUniformBlocks["UniformSubBlocks"]) {
								std::string subBlockName = jUniformSubBlocks["SubBlockName"];
								std::string DefineConnection = jUniformSubBlocks["DefineConnection"];

								ParamSubBlock* subBlock = new ParamSubBlock();
								subBlock->Name = subBlockName;
								subBlock->Define = DefineConnection;

								if (DefineConnection.length() != 0) {
									def->setDefine(DefineConnection, false);
								}

								for (jjson jUniforms : jUniformSubBlocks["Uniforms"]) {
									std::string uType = jUniforms["Type"];
									std::string uName = jUniforms["Name"];
									std::string uShaderName = jUniforms["ShaderName"];
									VarType uVarType = getVarTypeByString(uType);

									MaterialParam* param = nullptr;

									switch (uVarType) {
										case VarType::Int:
											param = def->setParam<int>(uShaderName, 0);
											break;
										case VarType::Bool:
											param = def->setParam<bool>(uShaderName, false);
											break;
										case VarType::Float:
											param = def->setParam<float>(uShaderName, 0);
											break;
										case VarType::Vector2:
											param = def->setParam<glm::vec2>(uShaderName, glm::vec2(1.0f));
											break;
										case VarType::Vector3:
											param = def->setParam<glm::vec3>(uShaderName, glm::vec3(1.0f));
											break;
										case VarType::Vector4:
											param = def->setParam<glm::vec4>(uShaderName, glm::vec4(1.0f));
											break;
										case VarType::Matrix3:
											param = def->setParam<glm::mat3>(uShaderName, glm::mat3());
											break;
										case VarType::Matrix4:
											param = def->setParam<glm::mat4>(uShaderName, glm::mat4());
											break;
										case VarType::Texture:
											Texture* texVal = nullptr;
											if (jUniforms.find("Value") != jUniforms.end()) {
												texVal = loadTexture(jUniforms["Value"]);
											}
											param = def->setParam<Texture*>(uShaderName, texVal);
											break;
									}
									param->IsPreset = true;
									param->Label = uName;
									subBlock->Params.push_back(param);
								}
								block.SubBlocks.push_back(subBlock);
							}
							def->addBlock(block);
						}
					}

					def->loadShaders();
					materialDef->addTechnique(def);
				}

				if (materialDef->getCount() == 0) {
					throw std::runtime_error("Material " + materialPath + " has 0 shaders !");
				}

				Material* mat = new Material(materialDef);
				mat->setUseLight(true);
				mat->recompile();
				return mat;
			} else {
				throw std::runtime_error("Material \"" + materialPath + "\" has no config file !");
			}
		} catch (const std::exception& ex) {
			std::cerr << ex.what() << std::endl;
			throw;
		}
	}

	unsigned char* resizeBilinear(unsigned char* pixels, int w, int h, int w2, int h2) {
		unsigned char* temp = new unsigned char[w2 * h2 * 4];
		int a, b, c, d, x, y, index;
		float x_ratio = ((float)(w - 1)) / w2;
		float y_ratio = ((float)(h - 1)) / h2;
		float x_diff, y_diff, blue, red, green;
		int offset = 0;
		for (int i = 0; i<h2; i++) {
			for (int j = 0; j<w2; j++) {
				x = (int)(glm::floor(x_ratio * j));
				y = (int)((glm::floor(y_ratio * i)));
				x_diff = (x_ratio * j) - x;
				y_diff = (y_ratio * i) - y;
				index = (y*w + x);
				a = pixels[index];
				b = pixels[index + 1];
				c = pixels[index + 2];
				d = pixels[index + 3];
				
				temp[offset++] = a;
				temp[offset++] = b;
				temp[offset++] = c;
				temp[offset++] = d;
			}
		}
		return temp;
	}

	Texture* AssetManager::loadTexture(std::string path) {
		if (_textureCache.find(path) != _textureCache.end()) {
			std::cout << "Loaded from cache: " << path << std::endl;
			return static_cast<Texture2D*>(_textureCache[path]);
		} else if (File(path).getExtension() == "gif") {
			Texture* tex = loadGif(File(path));
			if (tex == nullptr) {
				return nullptr;
			}
			tex->setWrap(WrapMode::Repeat);
			tex->setMinMagFilter(MinFilter::Trilinear, MagFilter::Bilinear);
			tex->setFile(path, File(path).getName());
			MetaManager::loadTextureMeta(tex, File(path));
			if (_renderer) {
				_renderer->setTexture(tex, 0);
			}
			_textureCache[path] = tex;
			return tex;
		} else {
			int x, y, bytesPerPixel;
			File f(path);
			unsigned char* pixels = nullptr;
			
			pixels = stbi_load(path.c_str(), &x, &y, &bytesPerPixel, 4);

			if (pixels == NULL) {
				std::cerr << "Unable to load texture(" << stbi_failure_reason() << "): " << path << std::endl;
				return nullptr;
			}

			/*File cacheFolder = File("TestProject/cache");
			File cacheFile = File(cacheFolder, File(path).getName());
			if (!cacheFile.isExist()) {
				ColorBitmap bitmap(pixels, x, y);
				ColorBitmap resized(128, 128);
				bitmap.scale(&resized, resized.getWidth(), resized.getHeight());

				pixels = resized.getPixels();
				x = resized.getWidth();
				y = resized.getHeight();
				std::cout << cacheFile << std::endl;
				std::string ext = cacheFile.getExtension(true);
				if (ext == "png") {
					stbi_write_png(cacheFile.getPath().c_str(), x, y, 4, pixels, x * 4);
				} else if (ext == "tga") {
					stbi_write_tga(cacheFile.getPath().c_str(), x, y, 4, pixels);
				}
			}*/

			std::cout << "Texture " << path << " loaded." << std::endl;
			Texture2D* tex = new Texture2D(x, y, ImageFormat::RGBA8, pixels);
			tex->setBitsPerPixel(bytesPerPixel * 8);
			_textureCache[path] = tex;

			tex->setWrap(WrapMode::Repeat);
			tex->setMinMagFilter(MinFilter::Trilinear, MagFilter::Bilinear);
			tex->setFile(path, File(path).getName());

			//MetaManager::saveTextureMeta(tex, File(path));
			MetaManager::loadTextureMeta(tex, File(path));

			if (_renderer) {
				_renderer->setTexture(tex, 0);
			}

			return tex;
		}
	}

	void AssetManager::decodeFaces(unsigned char * faces, std::vector<void*>& faceList, int & w, int & h) {
		int tileWidth = w / 4;
		int tileHeight = h / 3;

		faceList.push_back(unpackPixels(tileWidth * 2, tileHeight, tileWidth, tileHeight, w, faces));
		faceList.push_back(unpackPixels(0, tileHeight, tileWidth, tileHeight, w, faces));

		faceList.push_back(unpackPixels(tileWidth, 0, tileWidth, tileHeight, w, faces));
		faceList.push_back(unpackPixels(tileWidth, tileHeight * 2, tileWidth, tileHeight, w, faces));

		faceList.push_back(unpackPixels(tileWidth, tileHeight, tileWidth, tileHeight, w, faces));
		faceList.push_back(unpackPixels(tileWidth * 3, tileHeight, tileWidth, tileHeight, w, faces));

		w = tileWidth;
		h = tileHeight;
	}

	unsigned char* AssetManager::unpackPixels(int startX, int startY, int w, int h, int ow, unsigned char * pixels) {
		unsigned char* rgbArray = new unsigned char[w * h * 4];
		for (int x = startX; x < startX + w; x++) {
			for (int y = startY; y < startY + h; y++) {
				for (int a = 0; a < 4; a++) {
					rgbArray[((y - startY) * w + (x - startX)) * 4 + a] = pixels[(y * ow + x) * 4 + a];
				}
			}
		}
		return rgbArray;
	}

	Texture* AssetManager::loadCubeMap(std::string path) {
		if (_textureCache.find(path) != _textureCache.end()) {
			std::cout << "Loaded from cache: " << path << std::endl;
			return static_cast<TextureCubeMap*>(_textureCache[path]);
		} else {
			int x, y, bytesPerPixel;
			unsigned char* pixels = stbi_load(std::string(path).c_str(), &x, &y, &bytesPerPixel, 4);
			if (pixels == NULL) {
				std::cerr << "Unable to load texture: " << path << std::endl;
				return nullptr;
			}

			std::vector<void*> faces;
			decodeFaces(pixels, faces, x, y);
			stbi_image_free(pixels);
			TextureCubeMap* tex = new TextureCubeMap(x, y, ImageFormat::RGBA8, faces);
			tex->setWrap(WrapMode::ClampToEdge);
			_textureCache[path] = tex;
			std::cerr << "Texture loded: " << path << std::endl;
			return tex;
		}
	}

	Texture* AssetManager::loadGif(const File& file) {
		gif::List<gif::Bitmap> gifData;
		gif::Reader(file.getPath()).read(gifData);

		int w = 0;
		int h = 0;
		std::vector<void*> pixels;
		std::vector<float> delays;
		for (int i = 0; i < gifData.size(); i++) {
			const gif::List<gif::Bitmap>::Frame* frame = gifData.getFrame(i);
			w = frame->mBitmap.mWidth;
			h = frame->mBitmap.mHeight;
			unsigned char* pixelData = new unsigned char[(int)frame->mBitmap.mWidth * (int)frame->mBitmap.mHeight * 4];
			int a = 0;
			for (gif::ColorA8u c : frame->mBitmap.mPixels) {
				pixelData[a++] = c.r;
				pixelData[a++] = c.g;
				pixelData[a++] = c.b;
				pixelData[a++] = c.a;
			}
			pixels.push_back(pixelData);
			delays.push_back((float)frame->mDelay);
		}
		if (pixels.size() == 0) {
			return nullptr;
		}

		return new Texture2DAnimated(w, h, ImageFormat::RGBA8, pixels, delays);
	}

	Texture* AssetManager::loadHDR(const File & file) {
		stbi_set_flip_vertically_on_load(true);
		int width, height, nrComponents;
		float *data = stbi_loadf(file.getPath().c_str(), &width, &height, &nrComponents, 0);
		stbi_set_flip_vertically_on_load(false);
		if (data) {
			Texture2D* tex = new Texture2D(width, height, ImageFormat::RGB16F, nullptr);
			tex->setMinMagFilter(MinFilter::Bilinear, MagFilter::Bilinear);
			tex->setWrap(WrapMode::ClampToEdge);
			_renderer->setTexture(tex, 0);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);
			return tex;
		}
		return nullptr;
	}

	void AssetManager::recompileShader(Shader* shader) {

	}

	Shader* AssetManager::loadShader(std::string vertexShader, std::string tessControlShader, std::string tessEvalShader, std::string geomShader, std::string fragShader, std::string compShader) {
		File vertexFile(vertexShader);
		File tessControlFile(tessControlShader);
		File tessEvalFile(tessEvalShader);
		File geomFile(geomShader);
		File fragFile(fragShader);
		File compFile(compShader);

		File shaderFolder = vertexFile.getParentFile();

		if ((!vertexFile.isExist() || !fragFile.isExist()) && !compFile.isExist()) {
			return nullptr;
		}

		Shader* shader = new Shader();
		if (compFile.isExist()) {
			shader->addShader(ProgramType::Compute, "", compFile);
		} else {
			shader->addShader(ProgramType::Vertex, "", vertexFile);
			if (tessControlFile.isExist()) {
				shader->addShader(ProgramType::TessControl, "", tessControlFile);
			}
			if (tessEvalFile.isExist()) {
				shader->addShader(ProgramType::TessEvaluation, "", tessEvalFile);
			}
			if (geomFile.isExist()) {
				shader->addShader(ProgramType::Geometry, "", geomFile);
			}
			shader->addShader(ProgramType::Fragment, "", fragFile);
		}

		if (!shader->compile()) {
			std::cerr << "Shader load error: " << shaderFolder << std::endl;
			//return nullptr;
		}

		/*if (shaderFolder.getName() == "AO") {
			shader->bindFragOutput(0, "gPosition");
			shader->bindFragOutput(1, "gNormal");
		}*/

		return shader;
	}

	Shader* AssetManager::loadShader(std::string vertexShader, std::string geomShader, std::string fragShader) {
		return loadShader(vertexShader, "", "", geomShader, fragShader, "");
	}

	Shader* AssetManager::loadShader(std::string vertexShader, std::string fragShader) {
		return loadShader(vertexShader, "", fragShader);
	}

	Shader* AssetManager::loadShaderFolder(std::string folder) {
		File shaderFolder(folder);
		File vertex(shaderFolder, "shader." + Shader::getShaderTypeExtensionName(ProgramType::Vertex));
		File tessControlShader(shaderFolder, "shader." + Shader::getShaderTypeExtensionName(ProgramType::TessControl));
		File tessEvalShader(shaderFolder, "shader." + Shader::getShaderTypeExtensionName(ProgramType::TessEvaluation));
		File geomShader(shaderFolder, "shader." + Shader::getShaderTypeExtensionName(ProgramType::Geometry));
		File fragShader(shaderFolder, "shader." + Shader::getShaderTypeExtensionName(ProgramType::Fragment));
		File compShader(shaderFolder, "shader." + Shader::getShaderTypeExtensionName(ProgramType::Compute));

		return loadShader(vertex.getPath(), tessControlShader.getPath(), tessEvalShader.getPath(), geomShader.getPath(), fragShader.getPath(), compShader.getPath());
	}

	Texture* AssetManager::loadRawHeightMap(const File& file, const FileType& type) {
		if (_textureCache.find(file.getPath()) != _textureCache.end()) {
			std::cout << "Loaded from cache: " << file << std::endl;
			return static_cast<Texture*>(_textureCache[file.getPath()]);
		} else {
			FILE* fd = fopen(file.getPath().c_str(), "rb");
			if (fd == NULL) {
				fprintf(stderr, "Error in ReadRAW: couldn't read file %s\n", file);
				return nullptr;
			}
			unsigned int width = 2048 * 2;
			unsigned int height = 2048 * 2;

			//width += 1;
			//height += 1;

			unsigned int nbyte = width * height;

			std::uint16_t* bytes = new std::uint16_t[nbyte];
			if (fread(bytes, sizeof(std::uint16_t), nbyte, fd) != nbyte) {
				fprintf(stderr, "Error in ReadRaw8: couldn't read %d float in file %s\n", nbyte, file);
				fclose(fd);
				return nullptr;
			}
			fclose(fd);

			float* heightMap = new float[width * height];

			int i = 0;
			for (int x = 0; x < width; x++) {
				for (int y = 0; y < height; y++) {
					//heightMap[x + y * width] = ((float)bytes[i++] + (float)bytes[i++] * 256.0f) / 65535.0f;
					heightMap[x + y * width] = (float)bytes[i++] / 65535.0f;
				}
			}

			delete[] bytes;
			Texture2D* tex = new Texture2D(width, height, ImageFormat::R32F, heightMap);
			tex->setFile(file.getPath(), file.getName());
			_renderer->setTexture(tex, 0);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, heightMap);
			_renderer->setTexture(nullptr, 0);
			//delete[] heightMap;

			_textureCache[file.getPath()] = tex;

			return tex;
		}
	}

	TextureCacheMap& AssetManager::getLoadedTextures() {
		return _textureCache;
	}

	void AssetManager::setRenderer(Renderer* renderer) {
		_renderer = renderer;
	}

	Renderer* AssetManager::getRenderer() {
		return _renderer;
	}

	void AssetManager::loadTiff(const char* filename, unsigned char *& data, int & width, int & height) {
		/*std::string err;
		std::vector<tinydng::DNGImage> images;
		std::vector<tinydng::FieldInfo> custom_field_lists;

		bool ret = tinydng::LoadDNG(filename, custom_field_lists, &images, &err);

		if (!err.empty()) {
			std::cerr << "Err: " << err << std::endl;
		}

		if (ret) {
			std::cout << "Tiff loaded" << std::endl;
		}*/


	}

	Texture* AssetManager::loadTiff(const File& file) {
		if (_textureCache.find(file.getPath()) != _textureCache.end()) {
			std::cout << "Loaded from cache: " << file.getPath() << std::endl;
			return static_cast<Texture*>(_textureCache[file.getPath()]);
		} else {
			GLuint id = ilutGLLoadImage((wchar_t*)file.getPath().c_str());
			if (id) {
				int width = ilGetInteger(IL_IMAGE_WIDTH);
				int height = ilGetInteger(IL_IMAGE_HEIGHT);
				int bpp = ilGetInteger(IL_IMAGE_BITS_PER_PIXEL);
				Texture* tex = new Texture2D(width, height, ImageFormat::Custom);
				tex->setFile(file.getPath(), file.getName());
				tex->setTextureId(id);
				tex->setNeedsUpdate(false);
				tex->setNeedsUpdateParams(true);
				_textureCache[file.getPath()] = tex;
				return tex;
			}
			/*TinyTIFFReaderFile* tiffr = NULL;
			tiffr = TinyTIFFReader_open(file.getPath().c_str());
			if (!tiffr) {
				std::cout << "ERROR reading (not existent, not accessible or no TIFF file)\n";
			} else {
				uint32_t width = TinyTIFFReader_getWidth(tiffr);
				uint32_t height = TinyTIFFReader_getHeight(tiffr);
				uint16_t* image = (uint16_t*)calloc(width*height, sizeof(uint16_t));
				TinyTIFFReader_getSampleData(tiffr, image, 0);
				if (TinyTIFFReader_wasError(tiffr)) {
					std::cout << "ERROR:" << TinyTIFFReader_getLastError(tiffr) << "\n";
					return nullptr;
				}

				if (!image) {
					std::cout << "ERROR: Cannot load tiff!" << "\n";
					return nullptr;
				}
				Texture* tex = new Texture2D(width, height, ImageFormat::RGBA16F);
				tex->setFile(file.getPath(), file.getName());
				_renderer->setTexture(tex, 0);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, width, height, 0, GL_RED, GL_FLOAT, image);
				_renderer->setTexture(nullptr, 0);
				free(image);
				_textureCache[file.getPath()] = tex;
				return tex;
			}
			TinyTIFFReader_close(tiffr);*/
			return nullptr;
		}
	}

	Mesh* AssetManager::processMesh(aiMesh* mesh, const aiScene * scene) {
		Mesh* nMesh = new Mesh();

		switch (mesh->mPrimitiveTypes) {
			case aiPrimitiveType_POINT:
			nMesh->setMode(Mode::Points);
			break;
			case aiPrimitiveType_LINE:
			nMesh->setMode(Mode::Lines);
			break;
			case aiPrimitiveType_TRIANGLE:
			nMesh->setMode(Mode::Triangles);
			break;
			case aiPrimitiveType_POLYGON:
			nMesh->setMode(Mode::Quads);
			break;
			default:
			break;
		}

		FloatBuffer* vertices = FloatBuffer::allocate(mesh->mNumVertices * 3);
		FloatBuffer* normals = FloatBuffer::allocate(mesh->mNumVertices * 3);
		FloatBuffer* tangents = nullptr;
		FloatBuffer* texData = nullptr;
		IntBuffer* indices = IntBuffer::allocate(mesh->mNumFaces * 3u);

		for (std::uint32_t vertIdx = 0u; vertIdx < mesh->mNumVertices; vertIdx++) {
			aiVector3D vert = mesh->mVertices[vertIdx];
			aiVector3D norm = mesh->mNormals[vertIdx];
			if (mesh->HasTangentsAndBitangents()) {
				if (tangents == nullptr) {
					tangents = FloatBuffer::allocate(mesh->mNumVertices * 3);
				}
				aiVector3D tan = mesh->mTangents[vertIdx];
				tangents->put(tan.x)->put(tan.y)->put(tan.z);
			}

			vertices->put(vert.x)->put(vert.y)->put(vert.z);
			normals->put(norm.x)->put(norm.y)->put(norm.z);

			if (mesh->mTextureCoords[0]) {
				if (texData == nullptr) {
					texData = FloatBuffer::allocate(mesh->mNumVertices * 2);
				}
				texData->put(mesh->mTextureCoords[0][vertIdx].x)->put(mesh->mTextureCoords[0][vertIdx].y);
			}
		}

		for (std::uint32_t faceIdx = 0u; faceIdx < mesh->mNumFaces; faceIdx++) {
			indices->put(mesh->mFaces[faceIdx].mIndices[0u]);
			indices->put(mesh->mFaces[faceIdx].mIndices[1u]);
			indices->put(mesh->mFaces[faceIdx].mIndices[2u]);
		}

		if (mesh->HasBones()) {

		}

		nMesh->setData(MeshBuffer::Type::Position, 3, vertices);
		nMesh->setData(MeshBuffer::Type::Normal, 3, normals);
		if (tangents != nullptr) {
			nMesh->setData(MeshBuffer::Type::Tangent, 3, tangents);
		}
		if (texData != nullptr) {
			nMesh->setData(MeshBuffer::Type::TexCoord, 2, texData);
		}
		nMesh->setData(MeshBuffer::Type::Index, 3, indices);
		return nMesh;
	}

	void AssetManager::processAnimations(const aiScene* scene, Spatial * m) {
		if (scene->HasAnimations()) {
			Animator* animator = new Animator();
			for (int x = 0; x < scene->mNumAnimations; x++) {
				std::string AnimName = scene->mAnimations[x]->mName.data;
				if (AnimName.length() == 0) {
					AnimName = "Unknown";
				}
				std::cout << "Animation: " << AnimName << std::endl;

				Animation anim;
				anim.name = AnimName;
				anim.duration = scene->mAnimations[x]->mDuration;
				anim.ticksPerSecond = scene->mAnimations[x]->mTicksPerSecond;

				for (int y = 0; y < scene->mAnimations[x]->mNumChannels; y++) {
					Animation::Channel channel;
					channel.name = scene->mAnimations[x]->mChannels[y]->mNodeName.data;
					if (channel.name.length() == 0) {
						channel.name = "Unknown";
					}
					for (int z = 0; z < scene->mAnimations[x]->mChannels[y]->mNumPositionKeys; z++) {
						channel.mPositionKeys.push_back(scene->mAnimations[x]->mChannels[y]->mPositionKeys[z]);
					}
					for (int z = 0; z < scene->mAnimations[x]->mChannels[y]->mNumRotationKeys; z++) {
						channel.mRotationKeys.push_back(scene->mAnimations[x]->mChannels[y]->mRotationKeys[z]);
					}

					for (int z = 0; z < scene->mAnimations[x]->mChannels[y]->mNumScalingKeys; z++) {
						channel.mScalingKeys.push_back(scene->mAnimations[x]->mChannels[y]->mScalingKeys[z]);
					}

					anim.channels.push_back(channel);
				}

				animator->currentAnim = 0;

				for (int z = 0; z < MAX_BONES; z++) {
					anim.boneTrans.push_back(glm::mat4(1.0f));
				}

				animator->animations.push_back(anim);

			}
			m->setAnimator(animator);
			animator->animations[animator->currentAnim].root.name = "rootBoneTreeNode";
		}
	}
}