#ifndef PAINTABLE_TEXTURE
#define PAINTABLE_TEXTURE

#include "Engine/Render/FrameBuffer.h"
#include "Engine/Loaders/AssetManager.h"
#include "Engine/Render/RenderManager.h"
#include "Engine/Render/SpriteBatch.h"

namespace NovaEngine {

	namespace Painter {
		enum class PaintMode {
			None,
			Init,
			Radius
		};
	}

	class PaintableTexture : public Texture2D {
	private:
		AssetManager* _assetManager;
		RenderManager* _renderManager;
		Camera* _2dCamera;
		Renderer* _renderer;
		FrameBuffer* _mainFb;
		FrameBuffer* _pongFb;
		Texture* _pongTexture;
		Material* _paintMat;
		SpriteBatch* _batch;
		SpriteBatch* _pongBatch;
		GLfloat* _pixelData;

		Painter::PaintMode _paintMode;
	public:
		~PaintableTexture();
		PaintableTexture(AssetManager* assetManager, RenderManager* renderManager, int width, int height);

		void doPaint();
		void doPaintRadius(int x, int y, float radius, float cutOff, float strength);

		float readHeight(int x, int y);

		glm::vec2 getClickPos(int x, int y);
	};
}

#endif // !PAINTABLE_TEXTURE