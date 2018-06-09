#include "Engine/Render/PaintableTexture.h"
#include "Engine/Render/Mesh/Shape.h"

namespace NovaEngine {
	PaintableTexture::~PaintableTexture() {
		delete[] _pixelData;
	}

	PaintableTexture::PaintableTexture(AssetManager* assetManager, RenderManager* renderManager, int width, int height) : Texture2D(width, height, ImageFormat::RGB32F), _paintMode(Painter::PaintMode::Init) {
		_assetManager = assetManager;
		_renderManager = renderManager;
		_renderer = _renderManager->getRenderer();
		_2dCamera = new Camera(width, height);
		_2dCamera->update();
		setMinMagFilter(MinFilter::Nearest, MagFilter::Nearest);

		_mainFb = new FrameBuffer(width, height);
		_mainFb->createDepthBuffer(ImageFormat::Depth, false);
		_mainFb->addColorTexture(this);

		_pongFb = new FrameBuffer(width, height);
		_pongTexture = _pongFb->createColorBuffer(getFormat(), true);
		_pongTexture->setMinMagFilter(MinFilter::Nearest, MagFilter::Nearest);
		_pongFb->createDepthBuffer(ImageFormat::Depth, false);

		_paintMat = assetManager->loadMaterial("Assets/Materials/TexturePainter");
		_batch = new SpriteBatch(_2dCamera, _renderManager, _paintMat);

		_pongBatch = new SpriteBatch(_2dCamera, _renderManager, _assetManager->loadMaterial("Assets/Materials/Texture"));

		_pixelData = new GLfloat[width * height * 4];

		doPaint();
	}

	void PaintableTexture::doPaint() {
		if(_paintMode == Painter::PaintMode::None) return;
		// Ping texture
		_renderer->setFrameBuffer(_mainFb);
		_renderer->clearColor(makeRGB(0, 0, 0));
		_renderer->clearScreen(true, true, false);

		_batch->drawTexture(_pongTexture, 0, 0, _2dCamera->getWidth(), _2dCamera->getHeight());
		

		// Pong texture
		_renderer->setFrameBuffer(_pongFb);
		_renderer->clearColor(makeRGB(0, 0, 0));
		_renderer->clearScreen(true, true, false);
		_pongBatch->drawTexture(this, 0, 0, _2dCamera->getWidth(), _2dCamera->getHeight());

		glReadPixels(0, 0, _2dCamera->getWidth(), _2dCamera->getHeight(), getGLFormat(getFormat()), getGLDataType(getFormat()), _pixelData);
		_renderer->setFrameBuffer(nullptr);

		_paintMode = Painter::PaintMode::None;

		ViewPort* mwp = _renderManager->getViewPort("MainPort");
		glViewport(0, 0, mwp->getWidth(), mwp->getHeight());
	}

	float PaintableTexture::readHeight(int x, int y) {
		return _pixelData[(x + y * _2dCamera->getWidth()) * 4] * 0.4f;
	}

	glm::vec2 PaintableTexture::getClickPos(int x, int y) {

	}

	void PaintableTexture::doPaintRadius(int x, int y, float radius, float cutOff, float strength) {
		Camera* mainCamera = _renderManager->getViewPort("MainPort")->getCamera();
		Ray ray = mainCamera->getRay(x, mainCamera->getHeight() - y);

		Mesh* mesh = new Box(glm::vec3(), 5, 0.1, 5);
		mesh->updateBounds();
		mesh->createCollisionData();

		GameObject* obj = new GameObject();
		obj->setMesh(mesh);
		//obj->setRotation(-90, 0, 0);
		//obj->setScale(10, 10, 1);
		obj->setLocation(5, 0, 5);

		CollisionResults results;
		obj->collideWithRay(ray, results);
		if (results.size() > 0) {
			glm::vec3 cpos = results.getClosestCollision().contactPoint;
			glm::vec2 wpos = glm::vec2(cpos.x, cpos.z) / 10.0f;
			glm::vec2 tpos = glm::vec2(getWidth(), getHeight()) * wpos;

			_paintMode = Painter::PaintMode::Radius;
			_paintMat->setInt("PaintMode", (int)_paintMode);
			_paintMat->setVector2("Cursor", tpos);
			_paintMat->setFloat("RadPaint.radius", radius);
			_paintMat->setFloat("RadPaint.cutoff", cutOff);
			_paintMat->setFloat("RadPaint.strength", strength);

			doPaint();
		}

		//delete obj;

	}
}