#ifndef MATERIAL_PREVIEW_H
#define MATERIAL_PREVIEW_H

namespace NovaEngine {
	class RenderManager;
	class Texture;
	class Material;
	class Renderer;
	class FrameBuffer;
	class GameObject;
	class Camera;

	class MaterialPreview {
	private:
		RenderManager* _rm;
		Material* _material;
		Renderer* _renderer;
		FrameBuffer* _fb;
		Texture* _output;
		GameObject* _obj;
		Camera* _cam;
	public:
		MaterialPreview(RenderManager* rm, Material* material);
		void render();
		Texture* getOutput();
	};
}

#endif // !MATERIAL_PREVIEW_H