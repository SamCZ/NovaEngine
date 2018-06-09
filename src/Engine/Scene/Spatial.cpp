#include "Engine/Scene/Spatial.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "Engine/Scene/GameObject.h"
#include "Engine/Material/Material.h"
#include "Engine/Utils/File.h"
#include "Engine/Scene/Camera.h"

#include "Engine/Light/AmbientLight.h"
#include "Engine/Light/DirectionalLight.h"
#include "Engine/Light/PointLight.h"
#include "Engine/Light/SpotLight.h"

namespace NovaEngine {
	Spatial::Spatial() : Spatial("Unknown") {
		_animator = nullptr;
	}

	Spatial::Spatial(std::string name) : _location(0, 0, 0), _rotation(0, 0, 0), _scale(1, 1, 1), _childs(), _useParentTransform(true) {
		_name = name;
		_renderOrder = RenderOrder::Opaque;
		_type = SpatialType::Node;
		_animator = nullptr;
		_isLightNode = false;
	}

	Spatial::~Spatial() {
		/*if (getType() == SpatialType::GameObject) {
			GameObject* obj = static_cast<GameObject*>(this);
			delete obj->getMesh();
		}
		for (Spatial* child : _childs) {
			delete child;
		}*/
		//_childs.clear();
		if (_animator && _animator != nullptr) {
			delete _animator;
			_animator = nullptr;
		}
	}

	void Spatial::setLocation(const glm::vec3 & location) {
		_location = location;
	}

	void Spatial::setRotation(const glm::vec3 & rotation) {
		_rotation = rotation;
	}

	void Spatial::setScale(const glm::vec3 & scale) {
		_scale = scale;
	}

	void Spatial::setLocation(float x, float y, float z) {
		_location.x = x;
		_location.y = y;
		_location.z = z;
	}

	void Spatial::setRotation(float x, float y, float z) {
		_rotation.x = x;
		_rotation.y = y;
		_rotation.z = z;
	}

	void Spatial::setScale(float x, float y, float z) {
		_scale.x = x;
		_scale.y = y;
		_scale.z = z;
	}

	glm::vec3 & Spatial::getLocation() {
		return _location;
	}

	glm::vec3 & Spatial::getRotation() {
		return _rotation;
	}

	glm::vec3 & Spatial::getScale() {
		return _scale;
	}

	void Spatial::addChild(Spatial * spatial) {
		spatial->_parent = this;
		_childs.push_back(spatial);
	}

	void Spatial::removeChild(Spatial * spatial) {
		_childs.erase(std::find(_childs.begin(), _childs.end(), spatial));
	}

	std::vector<Spatial*>& Spatial::getChilds() {
		return _childs;
	}

	bool Spatial::isChildExist(Spatial* spatial) {
		return std::find(_childs.begin(), _childs.end(), spatial) != _childs.end();
	}

	void Spatial::update() {
		for (Component* comp : _components) {
			comp->update();
		}
		for (Spatial* child : _childs) {
			child->update();
		}
	}

	Spatial* Spatial::clone(bool deep) {
		Spatial* spatial = nullptr;
		if (getType() == SpatialType::GameObject) {
			GameObject* thisObj = static_cast<GameObject*>(this);
			GameObject* obj = new GameObject(getName());
			obj->setMesh(thisObj->getMesh());
			obj->setMaterial(thisObj->getMaterial());
			spatial = obj;
		} else {
			spatial = new Spatial(getName());
		}
		spatial->setLocation(getLocation());
		spatial->setRotation(getRotation());
		spatial->setScale(getScale());
		if(getFileSource() != nullptr)
			spatial->setFileSource(File(getFileSource()->getPath()));
		spatial->setRenderOrder(getRenderOrder());
		for (Spatial* child : getChilds()) {
			spatial->addChild(child->clone(deep));
		}
		return spatial;
	}

	void Spatial::setParent(Spatial* spatial) {
		_parent = spatial;
	}

	Spatial * Spatial::getParent() {
		return _parent;
	}

	Spatial* Spatial::operator[](const int index) {
		return _childs[index];
	}

	int Spatial::getChildCount() const {
		return (int)_childs.size();
	}

	void Spatial::removeAllChildren() {
		_childs.clear();
	}

	void Spatial::setRenderOrder(RenderOrder order) {
		_renderOrder = order;
	}

	RenderOrder Spatial::getRenderOrder() const {
		return _renderOrder;
	}

	glm::vec3 Spatial::getWorldLocation() {
		if (_parent != nullptr) {
			return _location + _parent->_location;
		}
		return _location;
	}

	glm::vec3 Spatial::getWorldRotation() {
		if (_parent != nullptr) {
			return _rotation + _parent->_rotation;
		}
		return _rotation;
	}

	glm::vec3 Spatial::getWorldScale() {
		if (_parent != nullptr) {
			return _scale * _parent->_scale;
		}
		return _scale;
	}

	glm::vec3 Spatial::getParentLocation() {
		if (_parent != nullptr) {
			return _parent->_location;
		}
		return (glm::vec3)0;
	}

	glm::vec3 Spatial::getParentRotation() {
		if (_parent != nullptr) {
			return _parent->_rotation;
		}
		return (glm::vec3)0;
	}

	glm::vec3 Spatial::getParentScale() {
		if (_parent != nullptr) {
			return _parent->_scale;
		}
		return (glm::vec3)1;
	}

	glm::mat4 Spatial::getModelMatrix(bool useWorld) {
		glm::mat4 rotation = glm::mat4();

		glm::vec3 axisX = glm::vec3(1, 0, 0);
		glm::vec3 axisY = glm::vec3(0, 1, 0);
		glm::vec3 axisZ = glm::vec3(0, 0, 1);

		rotation *= glm::rotate(glm::radians(_rotation.z), axisZ);
		rotation *= glm::rotate(glm::radians(_rotation.y), axisY);
		rotation *= glm::rotate(glm::radians(_rotation.x), axisX);

		glm::mat4 mat = (glm::translate(_location) * rotation) * glm::scale(_scale);
		if (_parent != nullptr && useWorld && _useParentTransform) {
			return _parent->getModelMatrix() * mat;
		}
		/*if (_parent != nullptr) {
			if (Camera* camera = dynamic_cast<Camera*>(_parent)) {
				//return camera->getModelMatrix(false, false) * mat;
				glm::mat4 camLoc = glm::translate(camera->getLocation());
				return camLoc * (mat * glm::inverse(camera->getViewMatrix()));
			}
			return _parent->getModelMatrix() * mat;
		}*/
		return mat;
	}

	Spatial* Spatial::find(const std::string& name) {
		for (Spatial* child : getChilds()) {
			if (child->getName() == name) {
				return child;
			}
		}
		return nullptr;
	}

	void Spatial::preRender() {

	}

	void Spatial::setType(SpatialType type) {
		_type = type;
	}

	SpatialType Spatial::getType() const {
		return _type;
	}

	void Spatial::setVisible(bool visible) {
		_visible = visible;
	}

	bool& Spatial::isVisible() {
		/*if (_parent != nullptr && _useParentTransform) {
			return _visible && _parent->isVisible();
		}*/
		return _visible;
	}

	bool Spatial::isVisibleInTree() {
		if (_parent != nullptr && _useParentTransform) {
			return _visible && _parent->isVisibleInTree();
		}
		return _visible;
	}

	void Spatial::setMaterial(Material * material) {
		for (Spatial* child : _childs) {
			child->setMaterial(material);
		}
	}

	void Spatial::setName(std::string name) {
		_name = name;
	}

	std::string Spatial::getName() const {
		return _name;
	}

	void Spatial::setAnimator(Animator * animator) {
		_animator = animator;
	}

	Animator* Spatial::getAnimator() {
		return _animator;
	}

	void Spatial::addLight(Light* light) {
		_lights.push_back(light);
		light->_parent = this;
		_isLightNode = true;
	}

	void Spatial::removeLight(Light* light) {
		auto iter = std::find(_lights.begin(), _lights.end(), light);
		if(iter != _lights.end()) {
			_lights.erase(iter);
		}
		if (_lights.size() == 0) {
			_isLightNode = false;
		}
	}

	std::vector<Light*>& Spatial::getLights() {
		return _lights;
	}
	bool Spatial::isLightNode() const {
		return _isLightNode;
	}

	int Spatial::collideWithRay(Ray r, CollisionResults& results) {
		int count = 0;
		for (Spatial* sp : _childs) {
			count += sp->collideWithRay(r, results);
		}
		return count;
	}

	File* Spatial::getFileSource() {
		return _fileSource;
	}

	void Spatial::setFileSource(const File& file) {
		_fileSource = new File(file);
	}

	void Spatial::setTemp(bool temp) {
		_isTemp = temp;
	}

	bool Spatial::isTemp() const {
		return _isTemp;
	}

	void Spatial::addComponent(Component* component) {
		component->preInit(this);
		_components.push_back(component);
	}

	bool Spatial::removeComponent(Component* component) {
		auto i = std::find(_components.begin(), _components.end(), component);
		if (i != _components.end()) {
			_components.erase(i);
			return true;
		}
		return false;
	}

	std::vector<Component*>& Spatial::getComponents() {
		return _components;
	}

	void Spatial::setUseParentTransform(bool useParent) {
		_useParentTransform = useParent;
	}

	bool Spatial::isUsingParentTransform() const {
		return _useParentTransform;
	}
}