#ifndef META_MANAGER_H
#define META_MANAGER_H

#include "Engine/Utils/Files.h"
#include "glm/glm.hpp"
#include "Editor/FileTree.h"

namespace NovaEngine {

	class File;
	class Texture;
	class Material;
	class AssetManager;
	class GOBJMapper;

	class MetaManager {
	public:
		static jjson fromVec2(const glm::vec2& v2);
		static jjson fromVec3(const glm::vec3& v3);
		static jjson fromVec4(const glm::vec4& v4);

		static glm::vec2 toVec2(jjson& json);
		static glm::vec3 toVec3(jjson& json);
		static glm::vec4 toVec4(jjson& json);
	public:
		static void saveTextureMeta(Texture* tex, const File& file);
		static void loadTextureMeta(Texture* tex, const File& file);

		static void saveMaterialMeta(Material* material, const File& file);
		static Material* loadMaterialFromMeta(AssetManager* assetManager, const File& file, FileTree* tree);

		static GOBJMapper* loadGOBJMap(const File& file);
		static void saveGOBJMap(GOBJMapper* mapper, const File& file);
	};
}

#endif // !META_MANAGER_H