#include "Engine/Material/MaterialPreview.h"

#include "Engine/Material/Material.h"
#include "Engine/Render/RenderManager.h"
#include "Engine/Render/Texture2D.h"
#include "Engine/Render/FrameBuffer.h"
#include "Engine/Scene/GameObject.h"
#include "Engine/Render/Mesh/Shape.h"

namespace NovaEngine {
	MaterialPreview::MaterialPreview(RenderManager* rm, Material* material) {
		_rm = rm;
		_material = material;
		_renderer = rm->getRenderer();
		_fb = new FrameBuffer(512, 512);
		_output = _fb->createColorBuffer(ImageFormat::RGBA8, true);
		_fb->createDepthBuffer(ImageFormat::Depth, false);

		_obj = new GameObject();
		_obj->setMaterial(_material);
		_obj->setMesh(new Sphere(30, 30, 3.0, true, false));
		_obj->setRotation(-90, 0, 0);

		_cam = new Camera(512, 512);
		_cam->setFrustumPerspective(67, 1.0f, 0.1, 20.0);
		_cam->setLocation(0, 0, 5.5);
		_cam->update();
	}

	void MaterialPreview::render() {
		_renderer->setFrameBuffer(_fb);
		_renderer->clearColor(0.0, 0.0, 0.0, 0.0);
		_renderer->clearScreen(true, true, false);
		_rm->renderGameObject(_obj, _cam);
		_renderer->setFrameBuffer(nullptr);
		_renderer->setInvalidateShader();
		Camera* mainCam = _rm->getViewPort("MainPort")->getCamera();
		glViewport(0, 0, mainCam->getWidth(), mainCam->getHeight());
	}

	Texture* MaterialPreview::getOutput() {
		return _output;
	}
}