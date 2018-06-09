#include "Engine/Terrain/HeightStorage.h"
#include <iostream>

namespace NovaEngine {
	HeightChunk::~HeightChunk() {
		delete[] _heightData;
	}
	HeightChunk::HeightChunk(int size) {
		_size = size;
		_heightData = new float[size * size];
	}
	float HeightChunk::getHeight(int x, int z) const {
		//if(x < 0 || x > _size-1 || z < 0 || z > _size-1) return 0.0f;
		return _heightData[x + (z * _size)];
	}
	void HeightChunk::setHeight(int x, int z, float height) {
		//if (x < 0 || x > _size - 1 || z < 0 || z > _size - 1) return;
		_heightData[x + (z * _size)] = height;
	}





	HeightStorage::~HeightStorage() {
		for (std::map<long, HeightChunk*>::iterator i = _chunks.begin(); i != _chunks.end(); i++) {
			delete i->second;
		}
	}

	HeightStorage::HeightStorage(int chunkSize) : _chunks(), myNoise(), perlin() {
		_chunkSize = 64;
		myNoise.SetNoiseType(FastNoise::PerlinFractal);
		perlin.SetNoiseType(FastNoise::Perlin);
	}
	HeightChunk* HeightStorage::getChunk(int x, int z) {
		int hash = hash_ivec2(x, z);
		if (_chunks.find(hash) != _chunks.end()) {
			return _chunks[hash];
		} else {
			HeightChunk* chunk = new HeightChunk(_chunkSize);
			generateChunkData(chunk, x, z);
			_chunks[hash] = chunk;
			return chunk;
		}
	}

	float clamp(float val, float min, float max) {
		if (val < min) {
			return min;
		}
		if (val > max) {
			return max;
		}
		return val;
	}

	float smoothstep(float edge0, float edge1, float x) {
		// Scale, bias and saturate x to 0..1 range
		x = clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0);
		// Evaluate polynomial
		return x*x*(3 - 2 * x);
	}

	float getBiomeNoise(float x, float z) {
		static FastNoise biomePerlin;
		biomePerlin.SetNoiseType(FastNoise::Perlin);
		float biome = biomePerlin.GetNoise(x * 0.8, z * 0.8);
		return biome;
	}

	float Nnoise(float x, float z, FastNoise::NoiseType type = FastNoise::Perlin) {
		static FastNoise noise;
		noise.SetNoiseType(type);
		return noise.GetNoise(x, z);
	}

	float getRoughtNoiseWithBiome(float x, float z) {
		/*float biome = getBiomeNoise(x, z);
		float height = 0.0f;

		float scale = 0.7;
		float exp = 60;

		if (biome < 0.1) {
			height -= exp / 2.0f;
			exp = 0;
		}

		static FastNoise noise;
		noise.SetNoiseType(FastNoise::PerlinFractal);
		height += noise.GetNoise(x * scale, z * scale) * exp;
		return height;*/
		/*float e = (1.0f * Nnoise(x * 1, z * 1)) + (0.5f * Nnoise(x * 2, z * 2)) + (0.25f * Nnoise(x * 4, z * 4));
		e = glm::pow(e, 1.32f);
		if (glm::isnan(e)) {
			e = 0;
		}
		e += Nnoise(x, z, FastNoise::CubicFractal) + (Nnoise(x * 10, z * 10, FastNoise::CubicFractal) * 0.1f);
		return e * 60;*/

		FastNoise noise;
		noise.SetNoiseType(FastNoise::SimplexFractal);
		noise.SetFrequency(0.01);
		noise.SetFractalType(FastNoise::Billow);
		noise.SetFractalOctaves(8);
		noise.SetFractalLacunarity(2.0);
		noise.SetFractalGain(0.5);
		
		FastNoise noise2;
		noise2.SetNoiseType(FastNoise::Cellular);
		noise2.SetFrequency(0.02);
		noise2.SetInterp(FastNoise::Quintic);
		noise2.SetFractalType(FastNoise::FBM);
		noise2.SetFractalOctaves(5);
		noise2.SetFractalLacunarity(2.0);
		noise2.SetFractalGain(0.5);
		noise2.SetCellularDistanceFunction(FastNoise::Euclidean);
		noise2.SetCellularReturnType(FastNoise::Distance2Sub);
		
		float rocks = noise2.GetCellular(x * 10.1, z * 10.1);

		return ((noise.GetNoise(x * 0.1, z * 0.1)) * 120) + (rocks * 3);
	}

	float getSmoothNoiseWithBiome(float x, float z) {
		float h = 0;
		int bSize = 100;
		float scale = 1;
		h += getRoughtNoiseWithBiome(x + -1 * scale, z + 0 * scale);
		h += getRoughtNoiseWithBiome(x + 1 * scale, z + 0 * scale);
		h += getRoughtNoiseWithBiome(x + 0 * scale, z + -1 * scale);
		h += getRoughtNoiseWithBiome(x + 0 * scale, z + 1 * scale);

		return h / 4.0f;
	}

	void HeightStorage::generateChunkData(HeightChunk* chunk, int x, int z) {
		for (int i = 0; i < _chunkSize; i++) {
			for (int j = 0; j < _chunkSize; j++) {
				float xx = (float)((x * _chunkSize) + i);
				float zz = (float)((z * _chunkSize) + j);
				chunk->setHeight(i, j, getNoiseHeight(xx, zz));// getNoiseHeight(xx, zz)
			}
		}
	}

	float HeightStorage::getNoiseHeight(float x, float z) {
		float y = 0;

		float scale1 = 0.1f;
		float scale2 = 0.8f;
		float scale3 = 0.25f;

		int stages = 2;

		float mountainHeight = 0;

		y += myNoise.GetNoise((x)* scale1, (z)* scale1) * 150;
		y += myNoise.GetNoise((x)* scale2, (z)* scale2) * 30;

		float plains = perlin.GetNoise((x)* scale3, (z)* scale3);
		//plains = (plains + 1.0f) / 2.0f;
		//plains *= 100;
		plains = glm::pow(plains * 100.0f, 1.2f);
		if (glm::isnan(plains)) {
			plains = 0;
		}
		if (plains < 0) {
			plains = 0;
		}

		y += plains;

		y += myNoise.GetNoise((x)* scale2, (z)* scale2) * 30;


		float fallOffStart = 500;
		float fallOffEnd = 1000;
		float fallUnderWaterEnd = 1200;

		float d = glm::distance(glm::vec2(x, z), glm::vec2(0, 0));
		float fo = smoothstep(fallOffEnd, fallOffStart, d);

		float fw = smoothstep(fallUnderWaterEnd, fallOffEnd-100, d) * 40.0f;

		fo = 1.0f;
		fw = 0.0;

		return (y * fo) + fw;
	}

	float HeightStorage::getHeight(int x, int z) {
		int cx = x >> 6;
		int cz = z >> 6;

		HeightChunk* chunk = getChunk(cx, cz);

		return chunk->getHeight(x & (64-1), z & (64-1));
	}
	void HeightStorage::setHeight(int x, int y, float height) {
		int cx = x >> 6;
		int cz = y >> 6;

		HeightChunk* chunk = getChunk(cx, cz);

		//std::cout << x << " " << z << std::endl;

		chunk->setHeight(x & (64 - 1), y & (64 - 1), height);
	}
}