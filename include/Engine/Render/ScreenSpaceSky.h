#ifndef SCREEN_SPACE_SKY_H
#define SCREEN_SPACE_SKY_H

namespace NovaEngine {

	class Spatial;
	class AssetManager;
	class RenderManager;
	class Camera;
	class SpriteBatch;
	class DirectionalLight;

	class ScreenSpaceSky {
	private:
		SpriteBatch* _batch;
		DirectionalLight* _dirLight;
		Camera* _camera;
		float _time;
	public:
		ScreenSpaceSky(AssetManager* assetManager, RenderManager* renderManager, Spatial* scene, Camera* camera, Camera* guiCamera);

		void render(float time);
	};
}

#endif // !SCREEN_SPACE_SKY_H