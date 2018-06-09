#ifndef RENDERER_H
#define RENDERER_H

#include "Engine/Render/Mesh/Mesh.h"
#include "Engine/Render/Shader.h"
#include "Engine/Render/Texture2D.h"
#include "Engine/Render/TextureCubeMap.h"
#include "Engine/Render/Texture2DAnimated.h"
#include "Engine/Render/RenderState.h"
#include "Engine/Utils/ColorRGBA.h"
#include "Engine/Render/FrameBuffer.h"

namespace NovaEngine {

	class AssetManager;
	class ViewPort;
	class RenderManager;

	//PBR
	enum class PBRMapType {
		EnviromentFromHDR,
		IrradianceFromEnviroment,
		PrefilteredMapFromEnviroment
	};

	class Renderer {
	public:
		Renderer();
		~Renderer();

		void clearScreen(bool color, bool depth, bool stencil);
		void clearColor(float r, float g, float b, float a);
		void clearColor(ColorRGBA color);

		void setShader(Shader* shader);

		void applyRenderState(RenderState& state);

		//MESH RENDER
		void renderMesh(Mesh* mesh, int lod);
		void finishRenderMesh(Mesh* mesh);
		//MESH END RENDER

		void setTexture(Texture* texture, int unit);
		void clearActiveTextures();

		//FRAMEBUFFER
		void setFrameBuffer(FrameBuffer* fb);
		void bindFrameBuffer(FrameBuffer* fb);
		void updateFrameBuffer(FrameBuffer* fb);
		void updateFrameBufferAttachment(FrameBuffer* fb, RenderBuffer* rb);
		int convertAttachmentSlot(int slot);
		void updateRenderBuffer(FrameBuffer* fb, RenderBuffer* rb);
		void updateRenderTexture(FrameBuffer* fb, RenderBuffer* rb);
		void setReadDrawBuffers(FrameBuffer* fb);
		void checkFrameBufferError();

		void finish();

		TextureCubeMap* renderViewPortScene(AssetManager* assetManager, ViewPort* wp, const glm::vec3& pos, RenderManager* rm);

		//PBR

		TextureCubeMap* createPBRMapFromHDR(AssetManager* assetManager, Texture* texture, const PBRMapType& type);
		Texture* createBRDFLUT(AssetManager* assetManager);

		inline void setInvalidateShader() {
			_invalidateShaderState = true;
		}
	private:
		Shader* _activeShader = nullptr;
		GLuint _vertexArrayId;
		RenderState _activeRenderState;
		bool _firstStateInit;
		bool _invalidateShaderState;

		GLint _initialDrawBuf;
		GLint _initialReadBuf;

		GLuint _vertexAttribArrays[16];
		int _enabledVertexAttribArrays = 0;

		unsigned int _attachments[16];

		Mesh* _lastMesh = nullptr;

		//MESH RENDER
		void updateBufferData(VertexBuffer* vb);
		GLenum convertUsage(MeshBuffer::Usage usage);
		GLenum convertFormat(MeshBuffer::Format format);
		//MESH END RENDER
	};
}

#endif // !RENDERER_H