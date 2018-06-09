#ifndef MATERIAL_H
#define MATERIAL_H

#include <string>
#include "Engine/Render/Texture.h"
#include "Engine/Render/Shader.h"
#include "Engine/Utils/ColorRGBA.h"
#include "Engine/Material/Technique.h"
#include "Engine/Scene/Camera.h"
#include "Engine/Render/RenderState.h"

#include "Engine/Light/Lights.h"

namespace NovaEngine {

	class MaterialPreview;
	class RenderManager;

	class Material {
	public:
		Material(MaterialDef* def);
		~Material();

		bool alpha = false;

		void setTechnique(std::string name);
		void setTechnique(TechniqueDef* technique);
		TechniqueDef* getTechnique(std::string name = "Default");

		RenderState& getRenderState(std::string technique = "Default");
		void setRenderState(RenderState& state, std::string technique = "Default");

		void setUseMaterialColors(bool use);

		void setLights(std::vector<Light*>& lights);
		bool isShaderMatParamExist(std::string name);

		void setCamera(Camera* camera);
		void setInt(std::string name, int i);
		void setFloat(std::string name, const float& f);
		void setBool(std::string name, bool b);
		void setVector2(std::string name, glm::vec2 vec);
		void setVector3(std::string name, glm::vec3 vec);
		void setVector4(std::string name, glm::vec4 vec);
		void setMatrix3(std::string name, glm::mat3 mat);
		void setMatrix4(std::string name, glm::mat4 mat);
		void setTexture(std::string name, Texture* texture, bool visibleInEditor = true);

		void setParam(std::string name, const VarType& type, void* data);

		template<typename T>
		inline MaterialParam* setParam(std::string name, T data) {
			return _activeTechnique->setParam<T>(name, data);
		}

		void uploadParamsToGPU(Renderer* renderer);
		void render(Renderer* renderer, Mesh* mesh);

		void setUseLight(bool use);
		bool& isUsingLights();

		std::string getName() const;

		void setFilename(const std::string filename);
		std::string getFilename() const;

		File* getFileSource();
		void setFileSource(const File& file);

		void updatePreview(RenderManager* renderManager);

		Texture* getPreviewTexture();

		void setUpdatePreview();

		void recompile();
	private:
		std::string _filename;
		File* _fileSource = nullptr;
		MaterialDef* _techniques;
		TechniqueDef* _activeTechnique;
		bool _useLights;
		std::vector<PointLight*> _pointLightCache;
		std::vector<DirectionalLight*> _directionalLightCache;

		int _nextIndex = 0;
		MaterialPreview* _previewGenerator = nullptr;
		bool _needsUpdatePreview = true;
	};
}

#endif // !MATERIAL_H