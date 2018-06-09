#ifndef POST_FILTER
#define POST_FILTER

#include <random>

#include "Engine/Material/Material.h"
#include "Engine/Render/FrameBuffer.h"
#include "Engine/Render/SpriteBatch.h"
#include "Engine/Loaders/AssetManager.h"
#include "Engine/Utils/Utils.h"
#include "Engine/Scene/ParallelCamera.h"

namespace NovaEngine {
	class PostFilterMesh {
	private:
		glm::vec4 _bg;
		Camera* _camera;
		RenderManager* _renderManager;
		Renderer* _renderer;
		FrameBuffer* _fb;
		Texture* _output;
		Texture* _depthTex;
		SpriteBatch* _batch;
	public:
		inline PostFilterMesh(Camera* camera, RenderManager* renderManager, bool depthTex = false) {
			_camera = camera;
			_renderManager = renderManager;
			_renderer = renderManager->getRenderer();

			_bg = glm::vec4(0, 0, 0, 0);

			_fb = new FrameBuffer(camera->getWidth(), camera->getHeight());
			_depthTex = nullptr;
			if (depthTex) {
				_depthTex = _fb->createDepthBuffer(ImageFormat::Depth16, true);
			} else {
				_fb->createDepthBuffer(ImageFormat::Depth, false);
			}

			_output = nullptr;
		}

		inline void setDefaultOutput() {
			_output = addColorTexture(ImageFormat::RGBA8);
		}

		inline Texture* addColorTexture(ImageFormat format) {
			return  _fb->createColorBuffer(format, true);
		}

		inline ~PostFilterMesh() {
			delete _fb;
		}

		inline void setBackground(float r, float g, float b, float a) {
			_bg.x = r;
			_bg.y = g;
			_bg.z = b;
			_bg.w = a;
		}

		inline void setBackground(const ColorRGBA& color) {
			_bg.x = color.r;
			_bg.y = color.g;
			_bg.z = color.b;
			_bg.w = color.a;
		}

		inline void render(ViewPort* viewPort, std::string technique) {
			Camera* oldCam = viewPort->getCamera();
			viewPort->setCamera(_camera);

			_renderer->setFrameBuffer(_fb);
			_renderer->clearColor(_bg.x, _bg.y, _bg.z, _bg.w);
			_renderer->clearScreen(true, true, false);
			_renderManager->setForcedTechnique(technique);
			_renderManager->renderViewPort(viewPort);
			_renderManager->setForcedTechnique("Default");
			_renderer->setFrameBuffer(nullptr);

			viewPort->setCamera(oldCam);
		}

		inline Texture* getDefaultResult() {
			return _output;
		}

		inline Texture* getDepthTexture() {
			return _depthTex;
		}
	};

	class PostFilter2D {
	private:
		Material* _material;
		Camera* _camera;
		RenderManager* _renderManager;
		Renderer* _renderer;
		FrameBuffer* _fb;
		Texture* _output;
		SpriteBatch* _batch;
	public:
		inline PostFilter2D(Material* material, Camera* camera, RenderManager* renderManager) {
			_material = material;
			_camera = camera;
			_renderManager = renderManager;
			_renderer = renderManager->getRenderer();

			_fb = new FrameBuffer(camera->getWidth(), camera->getHeight());
			_fb->createDepthBuffer(ImageFormat::Depth, false);

			_batch = new SpriteBatch(camera, renderManager, material);
			_output = nullptr;
		}

		inline void setDefaultOutput() {
			_output = addColorTexture(ImageFormat::RGBA8);
		}

		inline Texture* addColorTexture(ImageFormat format) {
			return  _fb->createColorBuffer(format, true);
		}

		inline ~PostFilter2D() {
			delete _fb;
			delete _batch;
		}

		inline void setTexture(std::string name, Texture* texture) {
			_material->setTexture(name, texture);
		}

		inline void render() {
			_renderer->setFrameBuffer(_fb);
			_renderer->clearScreen(true, true, false);
			_renderer->clearColor(makeRGBA(0, 0, 0, 0));
			_batch->drawTexture(nullptr, 0, 0, _camera->getWidth(), _camera->getHeight());
			_renderer->setFrameBuffer(nullptr);
		}

		inline Texture* getDefaultResult() {
			return _output;
		}
	};

	class PostBlur {
	private:
		PostFilter2D* _vertialFilter;
		PostFilter2D* _horizontalFilter;
		Material* _material;
	public:
		inline PostBlur(AssetManager* assetManager, Camera* camera, RenderManager* renderManager) {
			_material = assetManager->loadMaterial("Assets/Materials/GaussianBlur");
			_material->setBool("Horizontal", false);
			_vertialFilter = new PostFilter2D(_material, camera, renderManager);
			_vertialFilter->setDefaultOutput();

			Material* horizMat = assetManager->loadMaterial("Assets/Materials/GaussianBlur");
			horizMat->setBool("Horizontal", true);
			_horizontalFilter = new PostFilter2D(horizMat, camera, renderManager);
			_horizontalFilter->setDefaultOutput();
			_horizontalFilter->setTexture("ColorMap", _vertialFilter->getDefaultResult());
		}

		inline void setInput(Texture* texture) {
			_vertialFilter->setTexture("ColorMap", texture);
		}

		inline void render() {
			_vertialFilter->render();
			_horizontalFilter->render();
		}

		inline Texture* getResult() {
			return _horizontalFilter->getDefaultResult();
		}
	};

	class PostBloom {
	private:
		ViewPort* _wp;
		PostFilterMesh* _defaultRender;
		Texture* _defaultTextue;
		PostFilterMesh* _lightMapRender;
		Texture* _lightMapTexture;
		PostBlur* _blur;
		PostFilter2D* _finalMix;
	public:
		inline PostBloom(AssetManager* assetManager, ViewPort* wp, Camera* guiCamera, RenderManager* rm) {
			_wp = wp;

			_defaultRender = new PostFilterMesh(wp->getCamera(), rm);
			_defaultTextue = _defaultRender->addColorTexture(ImageFormat::RGBA8);

			_lightMapRender = new PostFilterMesh(wp->getCamera(), rm);
			_lightMapTexture = _lightMapRender->addColorTexture(ImageFormat::RGBA8);

			_blur = new PostBlur(assetManager, guiCamera, rm);
			_blur->setInput(_lightMapTexture);

			Material* finalMat = assetManager->loadMaterial("Assets/Materials/BloomMix");
			finalMat->getRenderState().blendMode = BlendMode::Alpha;
			_finalMix = new PostFilter2D(finalMat, guiCamera, rm);
			_finalMix->setDefaultOutput();
			_finalMix->setTexture("SceneMap", _defaultTextue);
			_finalMix->setTexture("BloomMap", _blur->getResult());
		}

		inline void render() {
			_defaultRender->render(_wp, "Default");
			_lightMapRender->render(_wp, "Bloom");
			_blur->render();
			_finalMix->render();
		}

		inline Texture* getResult() {
			return _finalMix->getDefaultResult();
		}
	};

	class PostFilters {
	private:
		ViewPort* _viewPort;
		RenderManager* _rm;

		PostFilterMesh* _gBuffer;
		Texture* _gPosTex;
		Texture* _gNormTex;
		Texture* _gAlbSpecTex;

		PostFilter2D* _ssaoFilter;
	public:
		Material* _material;
		PostFilters(AssetManager* assetManager, ViewPort* wp, Camera* guiCamera, RenderManager* rm) : _viewPort(wp), _rm(rm) {
			_gBuffer = new PostFilterMesh(wp->getCamera(), rm);
			_gAlbSpecTex = _gBuffer->addColorTexture(ImageFormat::RGBA8);
			_gNormTex = _gBuffer->addColorTexture(ImageFormat::RGB16F);
			_gPosTex = _gBuffer->addColorTexture(ImageFormat::RGB32F_RGB);

			_gPosTex->setMinMagFilter(MinFilter::Nearest, MagFilter::Nearest);
			_gNormTex->setMinMagFilter(MinFilter::Nearest, MagFilter::Nearest);
			_gAlbSpecTex->setMinMagFilter(MinFilter::Nearest, MagFilter::Nearest);

			_gPosTex->setWrap(WrapMode::ClampToEdge);
			_gNormTex->wrapEnabled = false;

			_material = assetManager->loadMaterial("Assets/Materials/SSAO");

			_material->setTexture("gPosition", _gPosTex);
			_material->setTexture("gNormal", _gNormTex);
			_material->setTexture("gAlbedoSpec", _gAlbSpecTex);

			_material->setVector2("ScreenSize", glm::vec2(wp->getWidth(), wp->getHeight()));

			_material->setMatrix4("PerspectiveMatrix", wp->getCamera()->getProjectionMatrix());

			_material->setFloat("radius", 0.5);
			_material->setFloat("bias", 0.025);

			std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between 0.0 - 1.0
			std::default_random_engine generator;
			for (unsigned int i = 0; i < 64; ++i) {
				glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
				sample = glm::normalize(sample);
				sample *= randomFloats(generator);
				float scale = float(i) / 64.0;

				// scale samples s.t. they're more aligned to center of kernel
				scale = lerp(0.1f, 1.0f, scale * scale);
				sample *= scale;
				_material->setVector3("Samples[" + std::to_string(i) + "]", sample);
			}

			std::vector<glm::vec3> ssaoNoise;
			for (unsigned int i = 0; i < 25; i++) {
				glm::vec3 noise(
					randomFloats(generator) * 2.0 - 1.0,
					randomFloats(generator) * 2.0 - 1.0,
					0.0f);
				ssaoNoise.push_back(noise);
			}

			Texture* noise = new Texture2D(5, 5, ImageFormat::RGB16F);
			noise->setWrap(WrapMode::Repeat);
			noise->setMinMagFilter(MinFilter::Nearest, MagFilter::Nearest);
			noise->wrapEnabled = true;
			rm->getRenderer()->setTexture(noise, 0);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 5, 5, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			_material->setTexture("gNoise", noise);

			_ssaoFilter = new PostFilter2D(_material, guiCamera, rm);
			_ssaoFilter->setDefaultOutput();
		}

		void render() {
			//_gBuffer->render(_viewPort, "Default");
			//_ssaoFilter->render();
			_rm->renderViewPort(_viewPort);
		}

		inline Texture* getResult(SpriteBatch* batch) {
			//batch->drawTexture(_ssaoFilter->getDefaultResult(), 0, 0, _viewPort->getWidth(), _viewPort->getHeight());
			//return _finalMix->getDefaultResult();
			return nullptr;
		}
	};

	class PostSSAO {
	private:
		
		ViewPort* _viewPort;

		PostFilterMesh* _defaultRender;
		Texture* _defaultTextue;
		Texture* _defaulDepthTextue;

		PostFilterMesh* _ssaoRender;
		Texture* _positionTextue;
		Texture* _normalTextue;

		PostFilter2D* _ssaoFilter;
		PostBlur* _blur;
		PostFilter2D* _finalMix;

	public:
		Material* _material;
		inline PostSSAO(AssetManager* assetManager, ViewPort* wp, Camera* guiCamera, RenderManager* rm) {
			_material = assetManager->loadMaterial("Assets/Materials/SSAO");
			_viewPort = wp;

			_ssaoRender = new PostFilterMesh(wp->getCamera(), rm);
			_positionTextue = _ssaoRender->addColorTexture(ImageFormat::RGB16F);
			_positionTextue->setWrap(WrapMode::ClampToEdge);
			_positionTextue->setMinMagFilter(MinFilter::Nearest, MagFilter::Nearest);

			_normalTextue = _ssaoRender->addColorTexture(ImageFormat::RGB16F);
			_normalTextue->setWrap(WrapMode::ClampToEdge);
			_normalTextue->setMinMagFilter(MinFilter::Nearest, MagFilter::Nearest);
			_normalTextue->wrapEnabled = false;

			_ssaoFilter = new PostFilter2D(_material, guiCamera, rm);
			_ssaoFilter->setDefaultOutput();

			_material->setTexture("PositionMap", _positionTextue);
			_material->setTexture("NormalMap", _normalTextue);
			_material->setVector2("iResolution", glm::vec2(wp->getWidth(), wp->getHeight()));
			_material->setMatrix4("Projection", wp->getCamera()->getProjectionMatrix());


			std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between 0.0 - 1.0
			std::default_random_engine generator;
			for (unsigned int i = 0; i < 64; ++i) {
				glm::vec3 sample(
					randomFloats(generator) * 2.0 - 1.0,
					randomFloats(generator) * 2.0 - 1.0,
					randomFloats(generator)
				);
				sample = glm::normalize(sample);
				sample *= randomFloats(generator);
				float scale = (float)i / 64.0;
				scale = lerp(0.1f, 1.0f, scale * scale);
				sample *= scale;
				_material->setVector3("Samples[" + std::to_string(i) + "]", sample);
			}

			std::vector<glm::vec3> ssaoNoise;
			for (unsigned int i = 0; i < 16; i++) {
				glm::vec3 noise(
					randomFloats(generator) * 2.0 - 1.0,
					randomFloats(generator) * 2.0 - 1.0,
					0.0f);
				ssaoNoise.push_back(noise);
			}

			Texture* noise = new Texture2D(4, 4, ImageFormat::RGB16F);
			noise->setWrap(WrapMode::Repeat);
			noise->setMinMagFilter(MinFilter::Nearest, MagFilter::Nearest);
			noise->wrapEnabled = true;
			rm->getRenderer()->setTexture(noise, 0);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
			_material->setTexture("TexNoise", noise);

			_defaultRender = new PostFilterMesh(wp->getCamera(), rm, true);
			_defaultTextue = _defaultRender->addColorTexture(ImageFormat::RGBA8);
			_defaulDepthTextue = _defaultRender->getDepthTexture();

			_material->setTexture("DepthMap", _defaulDepthTextue);

			_blur = new PostBlur(assetManager, guiCamera, rm);
			_blur->setInput(_ssaoFilter->getDefaultResult());


			Material* finalMat = assetManager->loadMaterial("Assets/Materials/SSAOMix");
			finalMat->getRenderState().blendMode = BlendMode::Alpha;
			_finalMix = new PostFilter2D(finalMat, guiCamera, rm);
			_finalMix->setDefaultOutput();
			_finalMix->setTexture("SceneMap", _defaultTextue);
			_finalMix->setTexture("AOMap", _blur->getResult());
		}

		inline void render() {
			_defaultRender->render(_viewPort, "Default");
			_ssaoRender->render(_viewPort, "AO");
			_ssaoFilter->render();
			_blur->render();
			_finalMix->render();
		}

		inline Texture* getResult(SpriteBatch* batch) {
			return _finalMix->getDefaultResult();
		}
	};

	class ShadowPost {
	private:
		DirectionalLight* _dirLight;
		ParallelCamera* _shadowCam;
		ViewPort* _wp;
		Camera* _camera;
		RenderManager* _rm;

		PostFilterMesh* _depthRender;
		

		glm::mat4 _offsetMat;
	public:
		Texture* _depthTextue;
		float frustumSize = 100.0;
		inline ShadowPost(DirectionalLight* dirLight, AssetManager* assetManager, ViewPort* wp, Camera* camera, RenderManager* rm) {
			_dirLight = dirLight;
			_wp = wp;
			_camera = camera;
			_rm = rm;

			_shadowCam = new ParallelCamera(1024 * 2, 1024 * 2);
			_shadowCam->setFrustumSize(frustumSize);

			_depthRender = new PostFilterMesh(_shadowCam, rm, true);
			_depthTextue = _depthRender->getDepthTexture();
			_depthTextue->setWrap(WrapMode::Clamp);
			_depthTextue->setMinMagFilter(MinFilter::Nearest, MagFilter::Nearest);

			rm->setShadowMap(_depthTextue);

			_offsetMat = glm::mat4(
				glm::vec4(0.5f, 0.0f, 0.0f, 0.0f),
				glm::vec4(0.0f, 0.5f, 0.0f, 0.0f),
				glm::vec4(0.0f, 0.0f, 0.5f, 0.0f),
				glm::vec4(0.5f, 0.5f, 0.5f, 1.0f)
			);

		}

		inline void render() {
			_shadowCam->setFrustumSize(frustumSize);
			//_shadowCam->getLocation() = _wp->getCamera()->getLocation();
			_shadowCam->lookAtDir(-_dirLight->getDirection());
			_shadowCam->update();
			_rm->setShadowMatrix(_offsetMat * _shadowCam->getProjectionViewMatrix());
			_depthRender->render(_wp, "Shadows");
		}

		inline Texture* getDepthTexture() {
			return _depthTextue;
		}
	};

	class ToneMappingRender {
	private:
		ViewPort* _wp;
		RenderManager* _renderManager;
		Renderer* _renderer;
		AssetManager* _assetManager;
		SpriteBatch* _batch;

		PostFilterMesh* _sceneBuffer;
		PostFilter2D* _finalMix;
	public:
		inline ToneMappingRender(ViewPort* wp, RenderManager* renderManager, Renderer* renderer, AssetManager* assetManager, SpriteBatch* batch) {
			_wp = wp;
			_renderManager = renderManager;
			_renderer = renderer;
			_assetManager = assetManager;
			_batch = batch;

			_sceneBuffer = new PostFilterMesh(wp->getCamera(), renderManager);
			_sceneBuffer->setDefaultOutput();
			//_sceneBuffer->addColorTexture(ImageFormat::)

			Material* finalMat = assetManager->loadMaterial("Assets/Materials/Post/ToneMapping");

			_finalMix = new PostFilter2D(finalMat, batch->getCamera(), renderManager);
			_finalMix->setDefaultOutput();

			finalMat->setTexture("ColorMap", _sceneBuffer->getDefaultResult());
		}

		inline void render() {
			_sceneBuffer->render(_wp, "Default");
			_finalMix->render();
		}

		inline void showScreen() {
			_batch->drawTexture(_finalMix->getDefaultResult(), 0, 0, _wp->getWidth(), _wp->getHeight());
		}
	};

	class DefferedRendering {
	private:
		ViewPort* _wp;
		RenderManager* _renderManager;
		Renderer* _renderer;
		AssetManager* _assetManager;
		SpriteBatch* _batch;

		PostFilterMesh* _gBuffer;
		Texture* _gPosTex;
		Texture* _gNormTex;
		Texture* _gAlbSpecTex;

		PostFilter2D* _finalMix;
		Material* _finalMat;
	public:
		inline DefferedRendering(ViewPort* wp, RenderManager* renderManager, Renderer* renderer, AssetManager* assetManager, SpriteBatch* batch) {
			_wp = wp;
			_renderManager = renderManager;
			_renderer = renderer;
			_assetManager = assetManager;
			_batch = batch;

			_gBuffer = new PostFilterMesh(wp->getCamera(), renderManager);
			_gPosTex = _gBuffer->addColorTexture(ImageFormat::RGB16F);
			_gNormTex = _gBuffer->addColorTexture(ImageFormat::RGB16F);
			_gAlbSpecTex = _gBuffer->addColorTexture(ImageFormat::RGBA8);

			_gBuffer->setBackground(wp->getBackgroundColor());

			_finalMat = assetManager->loadMaterial("Assets/Materials/Deffered");
			_finalMat->getRenderState().blendMode = BlendMode::Alpha;
			_finalMat->setUseLight(true);
			_finalMix = new PostFilter2D(_finalMat, batch->getCamera(), renderManager);
			_finalMix->setDefaultOutput();

			_finalMat->setTexture("gPosition", _gPosTex);
			_finalMat->setTexture("gNormal", _gNormTex);
			_finalMat->setTexture("gAlbedoSpec", _gAlbSpecTex);
		}

		inline void render() {
			_gBuffer->render(_wp, "Deffered");
			_finalMat->setLights(_wp->getScene()->getLights());
			_finalMat->setVector3("ViewPos", _wp->getCamera()->getLocation());
			_finalMix->render();
		}

		inline void showRendering() {
			_batch->drawTexture(_finalMix->getDefaultResult(), 0, 0);
		}

	};
}

#endif // !POST_FILTER