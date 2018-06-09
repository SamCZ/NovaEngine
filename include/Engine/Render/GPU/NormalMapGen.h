#ifndef NORMAL_MAP_GEN_H
#define NORMAL_MAP_GEN_H

#include "Engine/Loaders/AssetManager.h"
#include "Engine/Render/Renderer.h"
#include "Engine/Render/Texture.h"

namespace NovaEngine {
	class NormalMapGen {
	private:
		Renderer* _renderer;
		float _strength;
		int _N;
		Texture* _normalMap;
		Shader* _shader;
	public:
		inline ~NormalMapGen() {}

		inline NormalMapGen(int N, Renderer* renderer, AssetManager* assetManager) : _N(N), _strength(0.0f), _renderer(renderer), _normalMap(nullptr) {
			_shader = assetManager->loadShaderFolder("Assets/Materials/Computing/NormalMap");
			_normalMap = new Texture2D(N, N, ImageFormat::Custom);
			renderer->setTexture(_normalMap, 0);
			glTexStorage2D(GL_TEXTURE_2D, (int)(log(N) / log(2)), GL_RGBA32F, N, N);
		}

		inline void render(Texture* heightMap) {
			_renderer->setInvalidateShader();
			_renderer->setShader(_shader);
			_renderer->setTexture(heightMap, 0);
			_shader->setUniform1i("displacementmap", 0);
			_shader->setUniform1i("N", _N);
			_shader->setUniform1f("normalStrength", _strength);

			glBindImageTexture(0, _normalMap->getTextureId(), 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F);
			glDispatchCompute(_N / 16, _N / 16, 1);
			glFinish();
			_renderer->setTexture(_normalMap, 0);

			_renderer->setShader(nullptr);
			_renderer->setInvalidateShader();
		}

		inline void setStrength(float strength) {
			_strength = strength;
		}

		inline float& getStrength() {
			return _strength;
		}

		inline Texture* getNormalMap() {
			return _normalMap;
		}
	};
}

#endif // !NORMAL_MAP_GEN_H