#include "Engine/Scene/GameObject.h"
#include "Engine/Collisions/BoundingBox.h"
#include "Engine/Collisions/BIHTree.h"

namespace NovaEngine {
	GameObject::GameObject() : Spatial() {
		setType(SpatialType::GameObject);
		_postprocessFlags = 0;
		_mesh = nullptr;
		_shadowMode = ShadowMode::None;
	}

	GameObject::GameObject(std::string name) : Spatial(name) {
		setType(SpatialType::GameObject);
		_postprocessFlags = 0;
		_mesh = nullptr;
		_shadowMode = ShadowMode::None;
	}

	GameObject::GameObject(std::string name, Material * material, Mesh * mesh) : GameObject(name) {
		_material = material;
		_mesh = mesh;
	}

	GameObject::~GameObject() {
		if (_mesh != nullptr) {
			delete _mesh;
		}
	}

	void GameObject::setMesh(Mesh * mesh) {
		_mesh = mesh;
	}

	void GameObject::setMaterial(Material * material) {
		_material = material;
		Spatial::setMaterial(material);
	}

	Mesh* GameObject::getMesh() {
		return _mesh;
	}

	Material* GameObject::getMaterial() {
		return _material;
	}

	int GameObject::collideWithRay(Ray r, CollisionResults& results) {
		if (!_mesh) return 0;
		BIHTree* ctree = _mesh->getCollider();
		if (ctree == nullptr) {
			_mesh->createCollisionData();
			ctree = _mesh->getCollider();
			if (ctree == nullptr) return 0;
		}
		if (_mesh->getBounds() == nullptr) {
			_mesh->updateBounds();
		}
		BoundingBox* bbox = _mesh->getBounds()->transform(getWorldLocation(), getWorldRotation(), getWorldScale());
		CollisionResults local;
		int count = ctree->collideWithRay(r, this->getModelMatrix(), bbox, local);
		delete bbox;
		for (int i = 0; i < local.size(); i++) {
			CollisionResult res = local.getCollisonDirect(i);
			res.obj = this;
			results.addCollision(res);
		}
		return count;
	}
	void GameObject::addPostProcessFlag(int flag) {
		_postprocessFlags |= flag;
	}

	void GameObject::removePostProcessFlag(int flag) {
		_postprocessFlags &= ~flag;
	}

	bool GameObject::hasPostProcessFlag(int flag) {
		return (_postprocessFlags & flag) == flag;
	}

	void GameObject::setShadowMode(const ShadowMode & mode) {
		_shadowMode = mode;
	}

	ShadowMode GameObject::getShadowMode() const {
		return _shadowMode;
	}
}