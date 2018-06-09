#ifndef TERRAIN_QUAD
#define TERRAIN_QUAD

#include "Engine/Scene/GameObject.h"
#include "Engine/Terrain/HeightStorage.h"

namespace NovaEngine {
	class TerrainQuad : public GameObject {
	private:
		HeightStorage* _heightStorage;
	public:
		TerrainQuad(HeightStorage* heightStorage, Material* mat, Spatial* scene, float x, float z);

		void updateData();
	};
}

#endif // !TERRAIN_QUAD