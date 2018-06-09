#ifndef UI_EDITOR_H
#define UI_EDITOR_H

#include "Engine/NovaEngine.h"
#include "Engine/Render/Shader.h"
#include "Engine/Render/Mesh/Mesh.h"
#include "Engine/Render/Renderer.h"
#include "Engine/Loaders/AssetManager.h"
#include "Engine/Control/FirstPersonControl.h"

#include "al.h"

#include "Engine/Utils/TangentGenerator.h"
#include "Engine/Post/PostFilter.h"

using namespace NovaEngine;

class DoorObject {
public:
	GameObject* _obj;
	bool _sw;
	float _val;
public:
	inline DoorObject(float x, float y, float z, Material* mat) {
		Mesh* mesh = new Box(glm::vec3(), 1, 2, 0.1);
		mesh->updateBounds();
		mesh->createCollisionData();
		TangentGenerator::generateTangentAndBitangents(mesh);

		_obj = new GameObject();
		_obj->setMesh(mesh);
		_obj->setMaterial(mat);
		_obj->setLocation(x, y, z);

		_obj->setName("Door");

		_val = false;
		_val = 0;
	}

	inline void update(float delta) {
		if (_sw) {
			if (_val < 1.0f) {
				_val += 0.001f * delta;
				_obj->setRotation(0, _val * 90, 0);
			} else if (_val > 1.0f) {
				_val = 1.0f;
			}
		} else {
			if (_val > 0) {
				_val -= 0.001f * delta;
				_obj->setRotation(0, _val * 90, 0);
			} else if (_val < 0) {
				_val = 0;
			}
		}
	}

	inline void addToScene(Spatial* spatial) {
		spatial->addChild(_obj);
	}

	inline Ray doOpenClose(int x, int y, std::vector<DoorObject*> doors, Camera* camera) {
		Ray ray = camera->getRay(x, y);
		CollisionResults results;
		for (DoorObject* door : doors) {
			door->_obj->collideWithRay(ray, results);
		}
		if (results.size() > 0) {
			if (results.getClosestCollision().obj != _obj) {
				return ray;
			}
			if (_val == 0.0f || _val == 1.0f) {
				_sw = !_sw;
			}
		}
		return ray;
	}
};

class UIEditor : public Engine {
private:
	PostBloom* _bloomFilter;
	SpriteBatch* _cloudBatch;
	float time;
	Material* mat;

	std::vector<DoorObject*> doors;
	bool _mouseState;
	Ray rayToDraw;
	int collisions;
	CollisionResult ress;
	DirectionalLight* dirLight;
	PointLight* light;
	Material* grsMat;

	int _inspectorMode = -1;

	ShadowPost* _shadows;
public:
	Spatial* _selectedObj;

	void init() override;
	void update() override;
	void render() override;

	void renderUI() override;
	void renderImGui() override;

	void onMouseMove(int x, int y, int dx, int dy) override;
	void onMouseEvent(ButtonEvent e);
	void onKeyEvent(KeyEvent e);
};

#endif // !UI_EDITOR_H