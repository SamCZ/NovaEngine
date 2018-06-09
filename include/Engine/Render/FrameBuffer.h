#ifndef FRAME_BUFFER_H
#define FRAME_BUFFER_H

#include <vector>
#include "Engine/Utils/NativeObject.h"
#include "Engine/Render/Texture2D.h"

namespace NovaEngine {

	static const int SLOT_UNDEF = -1;
	static const int SLOT_DEPTH = -100;
	static const int SLOT_DEPTH_STENCIL = -101;

	class RenderBuffer {
	public:
		Texture2D* tex;
		ImageFormat format;
		GLuint id = -1;
		int slot = SLOT_UNDEF;
		int face = -1;
		int layer = -1;
		inline RenderBuffer(){
			tex = nullptr;
			id = -1;
			slot = SLOT_UNDEF;
			face = -1;
			layer = -1;
		}
	};

	class FrameBuffer : public NativeObject {
	private:
		int _width;
		int _height;
		std::vector<RenderBuffer*> _colorBufs;
		RenderBuffer* _depthBuf;
		int _colorBufIndex;
		bool _srgb;
	public:
		FrameBuffer(int width, int height);
		~FrameBuffer();

		Texture2D* createColorBuffer(ImageFormat format, bool toTexture);
		Texture2D* createDepthBuffer(ImageFormat format, bool toTexture);

		void setDepthBuffer(ImageFormat format);
		void setColorBuffer(ImageFormat format);

		void setMultiTarget(bool enabled);
		bool isMultiTarget();

		void setTargetIndex(int index);
		int getTargetIndex();

		void setColorTexture(Texture2D* tex);
		void clearColorTargets();
		void addColorBuffer(ImageFormat format);
		void addColorTexture(Texture2D* tex);
		void setDepthTexture(Texture2D* tex);

		RenderBuffer* getColorBuffer(int index);
		int getNumColorBuffers() const;
		RenderBuffer* getDepthBuffer();

		int getWidth() const;
		int getHeight() const;
	};
}

#endif // !FRAME_BUFFER_H