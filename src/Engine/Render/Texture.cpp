#include "Engine/Render/Texture.h"
#include "NanoVG/stb_image.h"

namespace NovaEngine {

	GLuint getGLInternalFormat(ImageFormat format) {
		switch (format) {
			case NovaEngine::ImageFormat::RGB8:
			return GL_RGB8;
			case ImageFormat::BGRA8:
			return GL_RGBA8;
			case NovaEngine::ImageFormat::RGB16F:
			return GL_RGB16F;
			case NovaEngine::ImageFormat::RGBA16F:
			return GL_RGBA16F;
			case ImageFormat::RGB32F:
			return GL_RGB32F;
			case ImageFormat::RGBA32F:
			case ImageFormat::RGB32F_RGB:
			return GL_RGBA32F;
			case NovaEngine::ImageFormat::RGBA8:
			return GL_RGBA8;
			case NovaEngine::ImageFormat::Depth:
			return GL_DEPTH_COMPONENT;
			case NovaEngine::ImageFormat::Depth16:
			return GL_DEPTH_COMPONENT16;
			case NovaEngine::ImageFormat::Depth24:
			return GL_DEPTH_COMPONENT24;
			case NovaEngine::ImageFormat::Depth24Stencil8:
			return GL_DEPTH24_STENCIL8_EXT;
			case ImageFormat::CompressedRGB:
			return GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
			case ImageFormat::CompressedRGBA:
			return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			case ImageFormat::RG16F:
			return GL_RG16F;
			case ImageFormat::R16F:
			return GL_R16F;
			case ImageFormat::R32F:
			return GL_R32F;
			default:
			return 0;
		}
	}

	GLuint getGLFormat(ImageFormat format) {
		switch (format) {
			case NovaEngine::ImageFormat::RGB32F:
			case NovaEngine::ImageFormat::RGB16F:
			case NovaEngine::ImageFormat::RGB8:
			case ImageFormat::RGB32F_RGB:
			return GL_RGB;
			case ImageFormat::BGRA8:
			return GL_BGRA;
			case NovaEngine::ImageFormat::RGBA32F:
			case NovaEngine::ImageFormat::RGBA16F:
			case NovaEngine::ImageFormat::RGBA8:
			return GL_RGBA;
			case NovaEngine::ImageFormat::Depth:
			return GL_DEPTH_COMPONENT;
			case NovaEngine::ImageFormat::Depth16:
			return GL_DEPTH_COMPONENT;
			case NovaEngine::ImageFormat::Depth24:
			return GL_DEPTH_COMPONENT;
			case NovaEngine::ImageFormat::Depth24Stencil8:
			return GL_DEPTH_STENCIL_EXT;
			case ImageFormat::CompressedRGB:
			return GL_RGB;
			case ImageFormat::CompressedRGBA:
			return GL_RGBA;
			case ImageFormat::RG16F:
			return GL_RG;
			case ImageFormat::R16F:
			return GL_RED;
			case ImageFormat::R32F:
			return GL_RED;
			default:
			return 0;
		}
	}

	GLuint getGLDataType(ImageFormat format) {
		switch (format) {
			case NovaEngine::ImageFormat::RGB8:
			return GL_UNSIGNED_BYTE;
			case NovaEngine::ImageFormat::RGBA8:
			return GL_UNSIGNED_BYTE;
			case ImageFormat::BGRA8:
			return GL_UNSIGNED_BYTE;
			case NovaEngine::ImageFormat::RGB16F:
			return GL_FLOAT;
			case NovaEngine::ImageFormat::RGBA16F:
			return GL_FLOAT;

			case NovaEngine::ImageFormat::RGB32F:
			return GL_FLOAT;
			case NovaEngine::ImageFormat::RGBA32F:
			case ImageFormat::RGB32F_RGB:
			return GL_FLOAT;

			case NovaEngine::ImageFormat::Depth:
			return GL_UNSIGNED_BYTE;
			case NovaEngine::ImageFormat::Depth16:
			return GL_FLOAT;
			case NovaEngine::ImageFormat::Depth24:
			return GL_UNSIGNED_INT;
			case NovaEngine::ImageFormat::Depth24Stencil8:
			return GL_UNSIGNED_INT_24_8_EXT;
			case ImageFormat::CompressedRGB:
			return GL_UNSIGNED_BYTE;
			case ImageFormat::CompressedRGBA:
			return GL_UNSIGNED_BYTE;
			case ImageFormat::RG16F:
			return GL_FLOAT;
			case ImageFormat::R16F:
			return GL_FLOAT;
			case ImageFormat::R32F:
			return GL_FLOAT;
			default:
			return 0;
		}
	}

	Texture::Texture(const int width, const int height, ImageFormat format) : _isNeedsGenerateMipMaps(true) {
		this->width = width;
		this->height = height;
		_format = format;
		_bpp = -1;
	}

	Texture::Texture(const int width, const int height, ImageFormat format, void* pixels) : Texture(width, height, format) {
		this->pixels.push_back(pixels);
	}

	Texture::Texture(const int width, const int height, ImageFormat format, const std::vector<void*>& pixels) : Texture(width, height, format) {
		for (void* row : pixels) {
			this->pixels.push_back(row);
		}
	}

	void Texture::setSize(int w, int h) {
		width = w;
		height = h;
	}

	void Texture::freeData() {
		for (void* data : this->pixels) {
			//delete[] data;
			//stbi_image_free(data);
		}
	}

	Texture::~Texture() {
		glDeleteTextures(1, &this->textureId);
		freeData();
	}

	bool Texture::isTherePixels() const {
		return this->pixels.size() > 0;
	}

	void Texture::setTextureId(GLuint id) {
		this->textureId = id;
	}

	void Texture::setAnisotropicFilter(int mode) {
		this->anisotropicFilter = mode;
		setNeedsUpdateParams(true);
	}

	int& Texture::getAnisotropicFilter() {
		return this->anisotropicFilter;
	}

	bool Texture::isNeedsGenerateMipmaps() const {
		return this->_isNeedsGenerateMipMaps;
	}
	void Texture::setGenerateMipmaps(bool generate) {
		this->_isNeedsGenerateMipMaps = generate;
	}

	float Texture::getLodBias() const {
		return this->lodBias;
	}
	void Texture::setLodBias(float bias) {
		this->lodBias = bias;
		setNeedsUpdateParams(true);
	}

	void Texture::setMinMagFilter(MinFilter min, MagFilter mag) {
		minificationFilter = min;
		magnificationFilter = mag;
		setNeedsUpdateParams(true);
	}

	void Texture::setWrap(WrapMode mode) {
		this->setWrap(WrapAxis::S, mode);
		this->setWrap(WrapAxis::T, mode);
		setNeedsUpdateParams(true);
	}

	void Texture::setWrap(WrapAxis axis, WrapMode mode) {
		switch (axis) {
			case WrapAxis::S:
			this->wrapS = mode;
			break;
			case WrapAxis::T:
			this->wrapT = mode;
			break;
			case WrapAxis::R:
			this->wrapR = mode;
			break;
			case WrapAxis::ST:
			this->wrapS = mode;
			this->wrapT = mode;
			break;
			default:
			break;
		}
		setNeedsUpdateParams(true);
	}
	WrapMode Texture::getWrap(WrapAxis axis) const {
		switch (axis) {
			case WrapAxis::S:
			return this->wrapS;
			case WrapAxis::T:
			return this->wrapT;
			case WrapAxis::R:
			return this->wrapR;
			default:
			return WrapMode::Clamp;
		}
	}

	void Texture::setDepth(bool depth) {
		this->depth = depth;
	}

	bool Texture::isDepth() const {
		return this->depth;
	}

	void Texture::setBitsPerPixel(int bpp) {
		_bpp = bpp;
	}

	int Texture::getBitsPerPixel() const {
		return _bpp;
	}

	void* Texture::getData(int index) {
		return this->pixels[index];
	}

	void Texture::setData(void* data) {
		this->pixels.clear();
		this->pixels.push_back(data);
		setNeedsUpdate(true);
	}

	void Texture::setData(int index, void* data) {
		this->pixels.insert(this->pixels.begin() + index, data);
		setNeedsUpdate(true);
	}

	int Texture::getDataCount() const {
		return this->pixels.size();
	}

	GLuint Texture::getTextureId() const {
		return this->textureId;
	}

	bool Texture::isNeedsUpdate() const {
		return this->needsUpdate;
	}

	void Texture::setNeedsUpdate(bool state) {
		this->needsUpdate = state;
	}

	bool Texture::isNeedsUpdateParams() const {
		return _needsUpdateParameters;
	}

	void Texture::setNeedsUpdateParams(bool state) {
		_needsUpdateParameters = state;
	}

	void Texture::setFormat(ImageFormat format) {
		_format = format;
	}

	ImageFormat Texture::getFormat() const {
		return _format;
	}

	int Texture::getWidth() const {
		return this->width;
	}
	int Texture::getHeight() const {
		return this->height;
	}

	void Texture::setNormalMap(bool normalMap) {
		_isNormalMap = normalMap;
	}

	bool& Texture::isNormalMap() {
		return _isNormalMap;
	}

	void Texture::setFile(const std::string& file, const std::string& name) {
		_file = file;
		_name = name;
	}
	std::string Texture::getFile() const {
		return _file;
	}

	std::string Texture::getName() const {
		return _name;
	}
}