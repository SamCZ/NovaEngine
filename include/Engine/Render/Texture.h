#ifndef TEXTURE_H
#define TEXTURE_H

#include <vector>
#include "GL/glew.h"

namespace NovaEngine {
	enum class ImageFormat {
		RGB8,
		RGBA8,
		BGRA8,
		BGR8,
		RGB16F,
		RGBA16F,
		RGB32F,
		RGBA32F,
		RGB32F_RGB,
		R16F,
		Depth,
		Depth16,
		Depth24,
		Depth24Stencil8,
		CompressedRGB,
		CompressedRGBA,

		RG16F,
		R32F,
		Custom
	};

	GLuint getGLInternalFormat(ImageFormat format);
	GLuint getGLFormat(ImageFormat format);
	GLuint getGLDataType(ImageFormat format);

	enum class TextureType {
		Texture2D, CubeMap
	};

	enum class WrapAxis {
		S,
		T,
		ST,
		R
	};

	enum class MinFilter {
		Nearest = GL_NEAREST,
		Bilinear = GL_LINEAR,
		Trilinear = GL_LINEAR_MIPMAP_LINEAR
	};

	enum class MagFilter {
		Nearest = GL_NEAREST,
		Bilinear = GL_LINEAR
	};

	enum class WrapMode {
		Repeat = GL_REPEAT,
		MirroredRepeat = GL_MIRRORED_REPEAT,
		Clamp = GL_CLAMP,
		ClampToEdge = GL_CLAMP_TO_EDGE,
		MirrorClamp = GL_MIRROR_CLAMP_EXT
	};

	class Texture {
	private:
		GLuint textureId = -1;
		bool needsUpdate = true;
		int width;
		int height;
		std::vector<void*> pixels;
		WrapMode wrapS = WrapMode::Clamp;
		WrapMode wrapT = WrapMode::Clamp;
		WrapMode wrapR = WrapMode::Clamp;
		ImageFormat _format;
		int anisotropicFilter;
		bool generatedMipmaps;
		float lodBias = -0.4f;
		bool depth = false;
		bool _isNormalMap = false;
		bool _needsUpdateParameters = true;
		int _bpp;
		bool _isNeedsGenerateMipMaps;
		std::string _name;
		std::string _file;
	public:
		bool wrapEnabled = true;
		bool AnimationFrame = false;

		MinFilter minificationFilter = MinFilter::Bilinear;
		MagFilter magnificationFilter = MagFilter::Bilinear;

		Texture(const int width, const int height, ImageFormat format);
		Texture(const int width, const int height, ImageFormat format, void* pixels);
		Texture(const int width, const int height, ImageFormat format, const std::vector<void*>& pixels);
		virtual ~Texture();
		void freeData();

		bool isTherePixels() const;

		void* getData(int index);

		void setData(void*);
		void setData(int index, void* data);
		int getDataCount() const;

		GLuint getTextureId() const;
		void setTextureId(GLuint id);

		bool isNeedsUpdate() const;
		void setNeedsUpdate(bool state);

		bool isNeedsUpdateParams() const;
		void setNeedsUpdateParams(bool state);

		int getWidth() const;
		int getHeight() const;

		void setSize(int w, int h);

		void setAnisotropicFilter(int mode);
		int& getAnisotropicFilter();

		bool isNeedsGenerateMipmaps() const;
		void setGenerateMipmaps(bool generate);

		float getLodBias() const;
		void setLodBias(float bias);

		void setMinMagFilter(MinFilter min, MagFilter mag);

		void setWrap(WrapMode mode);
		void setWrap(WrapAxis axis, WrapMode mode);
		WrapMode getWrap(WrapAxis axis) const;

		void setFormat(ImageFormat format);
		ImageFormat getFormat() const;

		void setDepth(bool depth);
		bool isDepth() const;

		void setBitsPerPixel(int bpp);
		int getBitsPerPixel() const;

		void setNormalMap(bool normalMap);
		bool& isNormalMap();

		void setFile(const std::string& file, const std::string& name);
		std::string getFile() const;
		std::string getName() const;

		virtual TextureType getType() = 0;
	};
}

#endif // !TEXTURE_H