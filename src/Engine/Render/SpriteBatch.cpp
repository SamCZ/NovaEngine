#include "Engine/Render/SpriteBatch.h"
#include "Engine/Render/Mesh/Shape.h"

namespace NovaEngine {

	SpriteBatch::SpriteBatch(Camera* guiCamera, RenderManager* renderManager, Material* mat) {
		_camera = guiCamera;
		_renderManager = renderManager;
		_material = mat;

		mat->getTechnique()->setDefine("USE_TEXTURE", true);
		mat->getTechnique()->setDefine("USE_COLOR", false);

		_guiObj = new GameObject();
		_guiObj->setMesh(new Quad(1, 1, false));
		_guiObj->setMaterial(mat);

		_guiObj->setRenderOrder(RenderOrder::Gui);

		_material->getRenderState().blendMode = BlendMode::Alpha;
		_material->getRenderState().faceCull = FaceCull::Off;
		_material->getRenderState().depthTest = false;

		mat->setParam<glm::vec2>("Scale", glm::vec2(1.0f));
	}

	SpriteBatch::~SpriteBatch() {
		delete _guiObj;
	}

	void SpriteBatch::drawTexture(Texture* texture, int x, int y, int width, int height, const ColorRGBA& color, bool trasparent, bool flipY) {
		_guiObj->setLocation(x, y, 0);
		_guiObj->setScale(width, height, 1);
		if (texture != nullptr) {
			_material->setTexture("Texture", texture);
		}
		_material->getRenderState().blendMode = trasparent ? BlendMode::Alpha : BlendMode::Off;
		_material->setBool("UseAlpha", trasparent);
		_material->setBool("UseOpacity", false);
		_material->setBool("FlipY", flipY);
		_material->setVector4("Tint", color.toVec4());
		_renderManager->renderGameObject(_guiObj, _camera);
	}

	void SpriteBatch::drawTexture(Texture* texture, int x, int y, int width, int height, const ColorRGBA& color, bool trasparent) {
		drawTexture(texture, x, y, width, height, color, trasparent, false);
	}

	void SpriteBatch::drawTexture(Texture* texture, int x, int y, int width, int height, const ColorRGBA& color) {
		drawTexture(texture, x, y, width, height, color, true);
	}

	void SpriteBatch::drawTexture(Texture* texture, int x, int y, int width, int height) {
		drawTexture(texture, x, y, width, height, makeRGB(255, 255, 255));
	}

	void SpriteBatch::drawTexture(Texture* texture, int x, int y) {
		if(texture == nullptr) return;
		drawTexture(texture, x, y, texture->getWidth(), texture->getHeight());
	}

	Material* SpriteBatch::getMaterial() {
		return _material;
	}

	Camera* SpriteBatch::getCamera() {
		return _camera;
	}

	GameObject* SpriteBatch::getObj() {
		return _guiObj;
	}
}
