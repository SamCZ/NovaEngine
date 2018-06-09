#include "Engine/Render/FrameBuffer.h"
#include <string>
#include <iostream>

namespace NovaEngine {
	FrameBuffer::FrameBuffer(int width, int height) : _colorBufs() {
		_width = width;
		_height = height;
		_depthBuf = nullptr;
		_colorBufIndex = 0;
		_srgb = false;
	}

	FrameBuffer::~FrameBuffer() {
		
	}

	Texture2D* FrameBuffer::createColorBuffer(ImageFormat format, bool toTexture) {
		Texture2D* texture = nullptr;
		if (toTexture) {
			texture = new Texture2D(_width, _height, format);
			addColorTexture(texture);
		} else {
			setColorBuffer(format);
		}
		return texture;
	}

	Texture2D* FrameBuffer::createDepthBuffer(ImageFormat format, bool toTexture) {
		Texture2D* texture = nullptr;
		if (toTexture) {
			texture = new Texture2D(_width, _height, format);
			setDepthTexture(texture);
		} else {
			setDepthBuffer(format);
		}
		return texture;
	}

	void FrameBuffer::setDepthBuffer(ImageFormat format) {
		_depthBuf = new RenderBuffer();
		_depthBuf->slot = SLOT_DEPTH;
		_depthBuf->format = format;
	}

	void FrameBuffer::setColorBuffer(ImageFormat format) {
		RenderBuffer* buffer = new RenderBuffer();
		buffer->slot = 0;
		buffer->format = format;
		_colorBufs.clear();
		_colorBufs.push_back(buffer);
	}

	void FrameBuffer::setMultiTarget(bool enabled) {
		if (enabled) _colorBufIndex = -1;
		else _colorBufIndex = 0;
	}

	bool FrameBuffer::isMultiTarget() {
		return _colorBufIndex == -1;
	}

	void FrameBuffer::setTargetIndex(int index) {
		if (index < 0 || index >= 16)
			throw new std::exception("Target index must be between 0 and 16");
		if (_colorBufs.size() < index)
			throw new std::exception(("The target at " + std::to_string(index) + " is not set!").c_str());
		_colorBufIndex = index;
		markUpdate();
	}

	int FrameBuffer::getTargetIndex() {
		return _colorBufIndex;
	}

	void FrameBuffer::setColorTexture(Texture2D* tex) {
		clearColorTargets();
		addColorTexture(tex);
	}

	void FrameBuffer::clearColorTargets() {
		_colorBufs.clear();
	}

	void FrameBuffer::addColorBuffer(ImageFormat format) {
		RenderBuffer* buff = new RenderBuffer();
		buff->slot = _colorBufs.size();
		buff->format = format;
		_colorBufs.push_back(buff);
	}

	void FrameBuffer::addColorTexture(Texture2D* tex) {
		RenderBuffer* buff = new RenderBuffer();
		buff->slot = _colorBufs.size();
		buff->tex = tex;
		buff->format = tex->getFormat();
		_colorBufs.push_back(buff);
	}

	void FrameBuffer::setDepthTexture(Texture2D* tex) {
		RenderBuffer* buff = new RenderBuffer();
		buff->slot = SLOT_DEPTH;
		buff->tex = tex;
		buff->format = tex->getFormat();
		_depthBuf = buff;
	}

	RenderBuffer* FrameBuffer::getColorBuffer(int index) {
		return _colorBufs[index];
	}

	int FrameBuffer::getNumColorBuffers() const {
		return _colorBufs.size();
	}

	RenderBuffer* FrameBuffer::getDepthBuffer() {
		return _depthBuf;
	}

	int FrameBuffer::getWidth() const {
		return _width;
	}

	int FrameBuffer::getHeight() const {
		return _height;
	}
}