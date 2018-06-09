#ifndef HEIGHT_STORAGE_H
#define HEIGHT_STORAGE_H

#include <map>
#include "glm/glm.hpp"
#include "Engine/Utils/glm_utils.h"

#include "Engine/Utils/FastNoise.h"

namespace NovaEngine {
	class HeightChunk {
	private:
		float* _heightData;
		int _size;
	public:
		~HeightChunk();
		HeightChunk(int size);

		float getHeight(int x, int z) const;
		void setHeight(int x, int z, float height);
	};

	class HeightStorage {
	private:
		std::map<long, HeightChunk*> _chunks;
		int _chunkSize;
		FastNoise myNoise;
		FastNoise perlin;

		void generateChunkData(HeightChunk* chunk, int x, int z);
		float getNoiseHeight(float x, float z);
	public:
		~HeightStorage();
		HeightStorage(int chunkSize);

		HeightChunk* getChunk(int x, int z);

		float getHeight(int x, int z);
		void setHeight(int x, int y, float height);
	};
}

#endif // !HEIGHT_STORAGE_H