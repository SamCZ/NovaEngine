#include "Engine/Render/ScreenSpaceSky.h"

#include "Engine/Loaders/AssetManager.h"
#include "Engine/Scene/Spatial.h"
#include "Engine/Scene/Camera.h"
#include "Engine/Render/RenderManager.h"
#include "Engine/Light/Lights.h"
#include "Engine/Render/SpriteBatch.h"

#include "glm/glm.hpp"

namespace NovaEngine {
	ScreenSpaceSky::ScreenSpaceSky(AssetManager* assetManager, RenderManager* renderManager, Spatial* scene, Camera* camera, Camera* guiCamera) {
		_camera = camera;
		_time = 0;

		_batch = new SpriteBatch(guiCamera, renderManager, assetManager->loadMaterial("Assets/Materials/Clouds"));
		_batch->getMaterial()->setVector2("view_port", glm::vec2(camera->getWidth(), camera->getHeight()));
		_batch->getMaterial()->setMatrix4("inv_proj", glm::inverse(camera->getProjectionMatrix()));
	}

	void ScreenSpaceSky::render(float time) {
		if (_dirLight != nullptr) {
			_batch->getMaterial()->setVector3("LightDir", glm::normalize(_dirLight->getDirection()));
		}
		_batch->getMaterial()->setFloat("iGlobalTime", _time);
		_batch->getMaterial()->setMatrix4("inv_view", glm::inverse(_camera->getViewMatrix()));
		_batch->drawTexture(nullptr, 0, 0, _camera->getWidth(), _camera->getHeight());
	}
}