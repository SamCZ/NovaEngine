#include "Editor/Scene.h"

namespace NovaEngine {
	Scene::Scene(Spatial* scene, std::string filename) {
		_scene = scene;
		_filename = filename;
	}

	Spatial * Scene::getScene() {
		return _scene;
	}

	std::string Scene::getFilename() {
		return _filename;
	}
}