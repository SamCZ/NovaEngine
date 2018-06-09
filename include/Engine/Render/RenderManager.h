#ifndef RENDER_MANAGER_H
#define RENDER_MANAGER_H

#include <vector>
#include "Engine/Render/Renderer.h"
#include "Engine/Scene/Camera.h"
#include "Engine/Scene/GameObject.h"
#include "Engine/Scene/ViewPort.h"
#include "Engine/Render/Postprocess/PostProcess.h"

namespace NovaEngine {
	class AssetManager;

	class PostFilterMesh;
	class PostFilter2D;
	class PostBlur;
	class SpriteBatch;

	class RenderManager {
	private:
		Renderer* _renderer;
		std::vector<ViewPort*> _viewPorts;
		std::vector<PostProcess*> _postProcessors;
		std::vector<GameObject*> _objToRender;
		std::string _forcedTechnique;
		TechniqueDef* _forcedTechniqueDef;
		glm::vec4 _clipPlane;
		std::vector<Light*> _lightsToRender;
		Texture* _shadowMap;
		glm::mat4 _shadowMatrix;

		bool _isPBRIBL;
		Texture* _irradianceMap;
		Texture* _prefilteredMap;
		Texture* _brdfLLUT;

		PostFilterMesh* _baseRender = nullptr;
		TextureCubeMap* _envMap = nullptr;
		Texture* _baseTexture = nullptr;
		Texture* _baseBloomTexture = nullptr;
		Texture* _baseDepthTexture = nullptr;
		Texture* _baseNormalTexture = nullptr;
		PostBlur* _baseBlur = nullptr;
		PostFilter2D* _basePostRender = nullptr;
	public:
		RenderManager(Renderer* renderer);
		~RenderManager();

		ViewPort* createViewPort(std::string name, Camera* camera, bool forcedSize);
		ViewPort* getViewPort(std::string name);

		void initPBR_IBL(AssetManager* assetManager);

		void resize(int w, int h);

		void renderViewPort(ViewPort* viewPort);
		void renderSpatial(Spatial* spatial, Camera* camera, RenderOrder order);
		void renderGameObject(GameObject* obj, Camera* camera);

		void prepareRenderSpatial(Spatial* spatial, Camera* camera, RenderOrder order);
		void prepareRenderGameObject(GameObject* obj, Camera* camera);
		void renderPreparedObjs();

		void renderViewPortWithFilters(ViewPort* viewPort, SpriteBatch* spriteBatch, AssetManager* assetManager);

		void addPostProcess(PostProcess* post);
		void removePostProcess(PostProcess* post);
		bool havePostProcess(PostProcess* post);

		void setForcedTechnique(std::string technique);
		void setForcedTechniqueDef(TechniqueDef* technique);

		void setClipPlane(glm::vec4 plane);

		void setShadowMap(Texture* tex);
		void setShadowMatrix(glm::mat4 m);

		TextureCubeMap* getEnviromentMap();

		Renderer* getRenderer();
	};
}

#endif // !RENDER_MANAGER_H