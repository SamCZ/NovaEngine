#include "Engine/Terrain/TerrainNode.h"
#include "Engine/Render/Mesh/Mesh.h"
#include "Engine/Utils/Buffers.h"
#include <iostream>
#include "Engine/Utils/FastNoise.h"

#include "Engine/Terrain/HeightStorage.h"

namespace NovaEngine {

	HeightStorage* storageTest = new HeightStorage(6000);

	void setVisibleS(Spatial* sp, bool visible) {
		for (Spatial* child : sp->getChilds()) {
			setVisibleS(child, visible);
		}
		sp->setVisible(visible);
	}

	void TerrainNode::update(glm::vec3 camera, float terrainMaxY) {
		float distance = glm::length(camera - glm::vec3(_worldPos.x, camera.y, _worldPos.z));
		//float distance = glm::length(glm::vec2(camera.x, camera.z) - glm::vec2(_worldPos.x, _worldPos.z));

		switch (_lod) {
			case 0: if (distance < _lodRanges[0]) {
				add4ChildNodes(_lod + 1);
			} else if (distance >= _lodRanges[0]) {
				removeChildNodes();
			}
			break;
			case 1: if (distance < _lodRanges[1]) {
				add4ChildNodes(_lod + 1);
			} else if (distance >= _lodRanges[1]) {
				removeChildNodes();
			}
			break;
			case 2: if (distance < _lodRanges[2]) {
				add4ChildNodes(_lod + 1);
			} else if (distance >= _lodRanges[2]) {
				removeChildNodes();
			}
			break;
			case 3: if (distance < _lodRanges[3]) {
				add4ChildNodes(_lod + 1);
			} else if (distance >= _lodRanges[3]) {
				removeChildNodes();
			}
			break;
			case 4: if (distance < _lodRanges[4]) {
				add4ChildNodes(_lod + 1);
			} else if (distance >= _lodRanges[4]) {
				removeChildNodes();
			}
			break;
			case 5: if (distance < _lodRanges[5]) {
				add4ChildNodes(_lod + 1);
			} else if (distance >= _lodRanges[5]) {
				removeChildNodes();
			}
			break;
			case 6: if (distance < _lodRanges[6]) {
				add4ChildNodes(_lod + 1);
			} else if (distance >= _lodRanges[6]) {
				removeChildNodes();
			}
			break;
			case 7: if (distance < _lodRanges[7]) {
				add4ChildNodes(_lod + 1);
			} else if (distance >= _lodRanges[7]) {
				removeChildNodes();
			}
			break;
		}

		for (Spatial* child : getChilds()) {
			TerrainNode* tree = (TerrainNode*)child;
			tree->update(camera, terrainMaxY);
		}
	}

	void TerrainNode::add4ChildNodes(int lod) {
		_isleaf = false;
		setVisible(false);
		if (getChildCount() == 0) {
			for (int i = 0; i<2; i++) {
				for (int j = 0; j<2; j++) {
					addChild(new TerrainNode(getMaterial(), _location + glm::vec2(i * _gap / 2.0f, j * _gap / 2.0f), lod, glm::ivec2(i, j)));
				}
			}
		}
	}

	void TerrainNode::removeChildNodes() {
		_isleaf = true;
		setVisible(true);

		for (Spatial* child : getChilds()) {
			TerrainNode* tree = (TerrainNode*) child;
			tree->removeChildNodes();
			delete child;
		}
		getChilds().clear();
	}

	FloatBuffer* TerrainNode::generatePatch(const glm::vec2& location, float gap) {
		FloatBuffer* fb = FloatBuffer::allocate(16 * 3);
		fb->put(location.x)->put(0.0)             ->put(location.y);
		fb->put(location.x + gap*0.333f)->put(0.0)->put(location.y);
		fb->put(location.x + gap*0.666f)->put(0.0)->put(location.y);
		fb->put(location.x + gap)->put(0.0)->put(location.y);

		fb->put(location.x)->put(0.0)->put(location.y + gap*0.333f);
		fb->put(location.x + gap*0.333f)->put(0)->put(location.y + gap*0.333f);
		fb->put(location.x + gap*0.666f)->put(0.0)->put(location.y + gap*0.333f);
		fb->put(location.x + gap)->put(0.0)->put(location.y + gap*0.333f);

		fb->put(location.x)->put(0.0)->put(location.y + gap*0.666f);
		fb->put(location.x + gap*0.333f)->put(0.0)->put(location.y + gap*0.666f);
		fb->put(location.x + gap*0.666f)->put(0.0)->put(location.y + gap*0.666f);
		fb->put(location.x + gap)->put(0.0)->put(location.y + gap*0.666f);

		fb->put(location.x)->put(0.0)->put(location.y + gap);
		fb->put(location.x + gap*0.333f)->put(0.0)->put(location.y + gap);
		fb->put(location.x + gap*0.666f)->put(0.0)->put(location.y + gap);
		fb->put(location.x + gap)->put(0.0)->put(location.y + gap);

		/*for (int i = 0; i < fb->size() / 3; i++) {
			float x = fb->get(i * 3 + 0);
			float y = fb->get(i * 3 + 1);
			float z = fb->get(i * 3 + 2);

			y = storageTest->getHeight(x * 100, z * 100);

			fb->put(i * 3 + 0, x);
			fb->put(i * 3 + 1, y);
			fb->put(i * 3 + 2, z);
		}*/

		return fb;
	}

	FloatBuffer* scaleBuffer(FloatBuffer* fb, float scale) {
		for (int i = 0; i < fb->size(); i++) {
			fb->put(i, fb->get(i) * scale);
		}
		return fb;
	}

	int updateMorphingArea(int lod, int rootPatches) {
		return (int)((TerrainQuadTree::TERRAIN_SIZE / rootPatches) / (glm::pow(2, lod)));
	}

	TerrainNode::TerrainNode(Material* mat, glm::vec2 location, int lod, glm::ivec2 index) : GameObject("TerrainNode") {
		setMaterial(mat);
		_isleaf = false;
		_location = location;
		_lod = lod;
		_index = index;
		_gap = 1.0f / (TerrainQuadTree::ROOT_PATCHES * (float)(glm::pow(2.0f, lod)));

		setTemp(true);
		setUseParentTransform(false);

		float scale = TerrainQuadTree::TERRAIN_SIZE;
		setScale(scale, 1.0, scale);

		glm::vec2 worldPos = ((location + (_gap / 2.0f)) * scale) - (scale / 2.0f);
		_worldPos = glm::vec3(worldPos.x, 0, worldPos.y);
		setLocation(-scale / 2.0f, 0, -scale / 2.0f);

		Mesh* mesh = new Mesh();
		mesh->setData(MeshBuffer::Type::Position, 3, generatePatch(_location, _gap));
		mesh->setPatchVertexCount(16);
		mesh->setMode(Mode::Patch);
		setMesh(mesh);
	}

	void TerrainNode::setMaterial(Material* material) {
		GameObject::setMaterial(material);
		for (Spatial* child : getChilds()) {
			child->setMaterial(material);
		}
		if (material == nullptr) return;
		_lodRanges[0] = 1750;
		_lodRanges[1] = 874;
		_lodRanges[2] = 386;
		_lodRanges[3] = 192;
		_lodRanges[4] = 100;
		_lodRanges[5] = 50;
		_lodRanges[6] = 0;
		_lodRanges[7] = 0;

		for (int i = 0; i < 8; i++) {
			_lodMorphArea[i] = _lodRanges[i] - updateMorphingArea(i + 1, TerrainQuadTree::ROOT_PATCHES);
		}

		for (int i = 0; i < 8; i++) {
			material->setInt("LodMorphArea[" + std::to_string(i) + "]", _lodMorphArea[i]);
		}
	}

	void TerrainNode::preRender() {
		Material* mat = getMaterial();
		if (mat == nullptr) return;

		mat->setInt("Lod", _lod);
		mat->setVector2("Index", _index);
		mat->setFloat("Gap", _gap);
		mat->setVector2("Location", _location);
	}

	TerrainNode::~TerrainNode() {

	}
}