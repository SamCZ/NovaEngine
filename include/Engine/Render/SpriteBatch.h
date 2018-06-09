#ifndef SPRITE_BATCH_H
#define SPRITE_BATCH_H

#include "Engine/Scene/Camera.h"
#include "Engine/Scene/GameObject.h"
#include "Engine/Render/RenderManager.h"
#include "Engine/Render/Texture.h"
#include "Engine/Material/Material.h"

namespace NovaEngine {
	class SpriteBatch {
	private:
		Camera* _camera;
		GameObject* _guiObj;
		Material* _material;
		RenderManager* _renderManager;

	public:
		SpriteBatch(Camera* guiCamera, RenderManager* renderManager, Material* mat);
		~SpriteBatch();

		void drawTexture(Texture* texture, int x, int y, int width, int height, const ColorRGBA& color, bool trasparent, bool flipY);
		void drawTexture(Texture* texture, int x, int y, int width, int height, const ColorRGBA& color, bool trasparent);
		void drawTexture(Texture* texture, int x, int y, int width, int height, const ColorRGBA& color);
		void drawTexture(Texture* texture, int x, int y, int width, int height);
		void drawTexture(Texture* texture, int x, int y);

		Material* getMaterial();
		Camera* getCamera();
		GameObject* getObj();
	};
}

#endif // !SPRITE_BATCH_H