#ifndef TERRAIN_QUAD_TREE
#define TERRAIN_QUAD_TREE

#include "Engine/Scene/GameObject.h"
#include "Engine/Utils/Buffers.h"
#include "Engine/Scene/ViewPort.h"
#include "Engine/Loaders/AssetManager.h"
#include "Engine/Render/GPU/NormalMapGen.h"

namespace NovaEngine {
	class TerrainNode : public GameObject {
	private:
		bool _isleaf;
		float _gap;
		int _lod;
		glm::vec2 _location;
		glm::ivec2 _index;
		glm::vec3 _worldPos;

		int _lodRanges[8];
		int _lodMorphArea[8];

		void add4ChildNodes(int lod);
		void removeChildNodes();

		static FloatBuffer* generatePatch(const glm::vec2& location, float gap);

		void preRender() override;
	public:
		TerrainNode(Material* mat, glm::vec2 location, int lod, glm::ivec2 index);
		~TerrainNode();

		void setMaterial(Material* material) override;

		void update(glm::vec3 camera, float terrainMaxY);
	};

	class TerrainQuadTree : public GameObject {
	private:
		Material* _material;
		ViewPort* _viewPort;
		AssetManager* _assetManager;
		NormalMapGen* _normalMapGen;
		float _lastScaleY;
		float _lastNormalStrength;
	public:
		static constexpr int ROOT_PATCHES = 8;
		static constexpr float TERRAIN_SIZE = 6000.0f;

		inline TerrainQuadTree(ViewPort* viewPort, AssetManager* assetManager, Material* mat) : GameObject("Terrain (CDLOD)"), _viewPort(viewPort), _assetManager(assetManager), _lastScaleY(-1.0f), _lastNormalStrength(24) {
			_material = mat;
			//_material->getRenderState().wireframe = true;
			setTemp(false);
			setFileSource(File("IMesh:TTClod"));

			for (int i = 0; i<ROOT_PATCHES; i++) {
				for (int j = 0; j<ROOT_PATCHES; j++) {
					addChild(new TerrainNode(_material, glm::vec2(1.0f * i / (float)ROOT_PATCHES, 1.0f * j / (float)ROOT_PATCHES), 0, glm::ivec2(i, j)));
				}
			}

			if (mat != nullptr) {
				MaterialParam* param = mat->getTechnique()->getParam("NormalMapStrength");
				if (param != nullptr) {
					_lastNormalStrength = param->getData<float>();
				}
			}

			_normalMapGen = new NormalMapGen(1081, _assetManager->getRenderer(), _assetManager);
			_normalMapGen->setStrength(_lastNormalStrength);
		}

		inline void setMaterial(Material* material) override {
			_material = material;
			GameObject::setMaterial(material);
			for (Spatial* child : getChilds()) {
				child->setMaterial(material);
			}
		}

		inline void update() override {
			if (_material != nullptr) {
				float nStrength = _material->getTechnique()->getParamData<float>("NormalMapStrength");
				if (_lastNormalStrength != nStrength) {
					_normalMapGen->setStrength(nStrength);
					_lastNormalStrength = nStrength;
					_lastScaleY = -1;
				}

				Texture* heightMap = _material->getTechnique()->getTextureParam("HeightMap");
				float scaleY = _material->getTechnique()->getParamData<float>("ScaleY");
				if (heightMap != nullptr && _lastScaleY != scaleY) {
					_normalMapGen->render(heightMap);
					_material->setTexture("HeightMapNormal", _normalMapGen->getNormalMap());
					_lastScaleY = scaleY;
				}
			}

			Camera* camera = _viewPort->getCamera();
			//float y = getHeight(camera->getLocation().x, camera->getLocation().z);
			for (Spatial* child : getChilds()) {
				TerrainNode* tree = (TerrainNode*)child;
				tree->update(camera->getLocation(), 0);
			}
		}

		inline float getHeight(float x, float z) {
			if (_material == nullptr) return 0;
			Texture* heightMap = _material->getTechnique()->getTextureParam("HeightMap");
			if (heightMap == nullptr) return 0;
			float scaleY = _material->getTechnique()->getParamData<float>("ScaleY");

			float h = 0.0;
			float scale = TerrainQuadTree::TERRAIN_SIZE;
			int res = heightMap->getWidth();
			
			glm::vec2 pos(x, z);
			pos += scale / 2.0f;
			pos /= scale;
			pos *= 1.0f;

			glm::vec2 posFloor(glm::floor(pos.x), glm::floor(pos.y));
			pos -= posFloor;
			pos *= res - 1;

			int x0 = (int)glm::floor(pos.x);
			int x1 = x0 + 1;
			int z0 = (int)glm::floor(pos.y);
			int z1 = z0 + 1;

			float* heightData = (float*)heightMap->getData(0);

			float h0 = heightData[res * z0 + x0];
			float h1 = heightData[res * z0 + x1];
			float h2 = heightData[res * z1 + x0];
			float h3 = heightData[res * z1 + x1];

			float percentU = pos.x - x0;
			float percentV = pos.y - z0;

			float dU, dV;
			if (percentU > percentV) {   // bottom triangle
				dU = h1 - h0;
				dV = h3 - h1;
			} else {   // top triangle
				dU = h3 - h2;
				dV = h2 - h0;
			}
			h = h0 + (dU * percentU) + (dV * percentV);

			h *= scaleY;

			//std::cout << h << std::endl;

			return h;
		}

		inline Material* getMaterial() {
			return _material;
		}
	};
}

#endif // !TERRAIN_QUAD_TREE