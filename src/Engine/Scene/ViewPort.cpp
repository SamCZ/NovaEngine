#include "Engine/Scene/ViewPort.h"

namespace NovaEngine {
	ViewPort::ViewPort(std::string name, Camera* camera) : _name(name), _forcedSize(false), _backgroundColor(makeRGB(64, 64, 64)) {
		_camera = camera;
		_scene = nullptr;
	}

	ViewPort::~ViewPort() {

	}

	Camera* ViewPort::getCamera() {
		return _camera;
	}

	void ViewPort::setCamera(Camera* camera) {
		_camera = camera;
	}

	void ViewPort::setBackgroundColor(ColorRGBA color) {
		_backgroundColor = color;
	}

	ColorRGBA& ViewPort::getBackgroundColor() {
		return _backgroundColor;
	}

	void ViewPort::setScene(Spatial* scene) {
		_scene = scene;
	}

	Spatial * ViewPort::getScene() {
		return _scene;
	}

	int ViewPort::getWidth() const {
		return _camera->getWidth();
	}

	int ViewPort::getHeight() const {
		return _camera->getHeight();
	}

	void ViewPort::setName(std::string name) {
		_name = name;
	}

	std::string ViewPort::getName() const {
		return _name;
	}

	void ViewPort::setForcedSize(bool forced) {
		_forcedSize = forced;
	}

	bool ViewPort::isForcedSize() const {
		return _forcedSize;
	}

}