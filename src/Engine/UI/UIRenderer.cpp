#include "Engine/UI/UIRenderer.h"

#include "GL/glew.h"
#define NANOVG_GL2_IMPLEMENTATION
#include "NanoVG/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "NanoVG/stb_image_write.h"
#include "NanoVG/nanovg_gl.h"

namespace NovaEngine {

	char* cpToUTF8(int cp, char* str) {
		int n = 0;
		if (cp < 0x80) n = 1;
		else if (cp < 0x800) n = 2;
		else if (cp < 0x10000) n = 3;
		else if (cp < 0x200000) n = 4;
		else if (cp < 0x4000000) n = 5;
		else if (cp <= 0x7fffffff) n = 6;
		str[n] = '\0';
		switch (n) {
			case 6: str[5] = 0x80 | (cp & 0x3f); cp = cp >> 6; cp |= 0x4000000;
			case 5: str[4] = 0x80 | (cp & 0x3f); cp = cp >> 6; cp |= 0x200000;
			case 4: str[3] = 0x80 | (cp & 0x3f); cp = cp >> 6; cp |= 0x10000;
			case 3: str[2] = 0x80 | (cp & 0x3f); cp = cp >> 6; cp |= 0x800;
			case 2: str[1] = 0x80 | (cp & 0x3f); cp = cp >> 6; cp |= 0xc0;
			case 1: str[0] = cp;
		}
		return str;
	}

	BorderRadius makeRadius(float topLeft, float topRight, float bottomRight, float bottomLeft) {
		BorderRadius r;
		r.topLeft = topLeft;
		r.topRight = topRight;
		r.bottomRight = bottomRight;
		r.bottomLeft = bottomLeft;
		return r;
	}
	BorderRadius makeRadius(float radius) {
		return makeRadius(radius, radius, radius, radius);
	}

	Border makeBorder(float thickness, const ColorRGBA& color, const BorderRadius& radius) {
		Border b;
		b.thickness = thickness;
		b.color = color;
		b.radius = radius;
		return b;
	}

	Border makeBorder(const BorderRadius& radius) {
		return makeBorder(0.0f, makeRGB(0, 0, 0), radius);
	}

	Border makeBorder() {
		return makeBorder(makeRadius(0.0f));
	}

	DropShadow makeShadow(float size, const ColorRGBA& color) {
		DropShadow ds;
		ds.size = size;
		ds.color = color;
		return ds;
	}

	DropShadow makeShadow() {
		return makeShadow(0.0f, makeRGBA(0, 0, 0, 0));
	}

	Paint makeColorPaint(ColorRGBA color) {
		Paint p;
		p.mode = PaintMode::Color;
		p.basicColor = color;
		return p;
	}

	Paint makeLinearGradientPaint(bool vertical, const ColorRGBA& startColor, const ColorRGBA& endColor) {
		Paint p;
		p.mode = PaintMode::LinearGradient;
		p.linearVertical = vertical;
		p.startColor = startColor;
		p.endColor = endColor;
		return p;
	}

	NVGcolor makeNVGColor(const ColorRGBA& color) {
		return nvgRGBAf(color.r, color.g, color.b, 1.0f);
	}

	UIRenderer::UIRenderer(Display* display) {
		_display = display;

		_context = nvgCreateGL2(NVG_ANTIALIAS);
		if (_context == NULL) {
			printf("Could not init nanovg.\n");
			return;
		}
		nvgCreateFont(_context, "Font", "Assets/Fonts/arial.ttf");
		nvgCreateFont(_context, "FontAwesome", "Assets/Fonts/fontawesome-webfont.ttf");

		_nextClipId = 0;

		_textBounds = new float[4];

		_selectedFont = "Font";
	}

	void UIRenderer::setFont(std::string font) {
		_selectedFont = font;
	}

	UIRenderer::~UIRenderer() {
		nvgDeleteGL2(_context);
	}

	void UIRenderer::begin() {
		nvgBeginFrame(_context, _display->getWidth(), _display->getHeight(), _display->getPixelRatio());
	}

	void UIRenderer::end() {
		nvgEndFrame(_context);
	}

	Display* UIRenderer::getDisplay() {
		return _display;
	}

	NVGcontext* UIRenderer::getContext() {
		return _context;
	}

	void UIRenderer::drawImage(Texture2D* tex, int x, int y, int width, int height, int borderRadius) {
		NVGpaint paint = nvgImagePattern(_context, (float)x, (float)y, (float)width, (float)height, 0.0f, getTextureNVGHandle(tex), 1.0f);
		nvgBeginPath(_context);
		if (borderRadius > 0) {
			nvgRoundedRect(_context, (float)x, (float)y, (float)width, (float)height, (float)borderRadius);
		} else {
			nvgRect(_context, (float)x, (float)y, (float)width, (float)height);
		}
		nvgFillPaint(_context, paint);
		nvgFill(_context);
	}

	int UIRenderer::getTextureNVGHandle(Texture2D* tex) {
		if (_textureNVGStore.find(tex->getTextureId()) != _textureNVGStore.end()) {
			return _textureNVGStore[tex->getTextureId()];
		}
		int tid = nvglCreateImageFromHandleGL2(_context, tex->getTextureId(), tex->getWidth(), tex->getHeight(), NVG_IMAGE_FLIPY);
		_textureNVGStore[tex->getTextureId()] = tid;
		return tid;
	}

	void UIRenderer::drawText(std::string text, int x, int y, float fontSize, ColorRGBA color, Align alignX, Align alignY) {
		nvgFontSize(_context, fontSize);
		nvgFontFace(_context, _selectedFont.c_str());

		int flags = 0;
		switch (alignX) {
			case Align::Left:
			flags |= NVG_ALIGN_LEFT;
			break;
			case Align::Right:
			flags |= NVG_ALIGN_RIGHT;
			break;
			case Align::Center:
			flags |= NVG_ALIGN_CENTER;
			break;
		}

		switch (alignY) {
			case Align::Top:
			flags |= NVG_ALIGN_TOP;
			break;
			case Align::Bottom:
			flags |= NVG_ALIGN_BOTTOM;
			break;
			case Align::Center:
			flags |= NVG_ALIGN_MIDDLE;
			break;
		}
		nvgFontBlur(_context, 0);
		nvgGlobalAlpha(_context, 1.0f);
		nvgTextAlign(_context, flags);
		nvgFillColor(_context, nvgRGBAf(color.r, color.g, color.b, color.a));
		nvgText(_context, (float)x, (float)y, text.c_str(), NULL);
		_selectedFont = "Font";
	}

	float UIRenderer::getTextSize(std::string text, int fontSize, int* textHeight) {
		nvgFontSize(_context, (float)fontSize);
		nvgFontFace(_context, "Font");
		nvgTextAlign(_context, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
		
		float w = nvgTextBounds(_context, 0.0f, 0.0f, text.c_str(), NULL, _textBounds);
		if (textHeight) {
			*textHeight = (int)(_textBounds[3] - _textBounds[1]);
		}
		return w;
	}

	/*void UIRenderer::drawRect(int x, int y, int w, int h, ColorRGBA color) {
		drawRect(x, y, w, h, color, false);
	}*/

	void UIRenderer::drawRect(int x, int y, int w, int h, ColorRGBA color, bool isStroke) {
		drawRect(x, y, w, h, color, isStroke, 1.0f);
	}


	void UIRenderer::drawRect(int x, int y, int w, int h, ColorRGBA color, bool isStroke, float strokeSize) {
		drawRect(x, y, w, h, color, isStroke, strokeSize, 0.0f);
	}

	void UIRenderer::drawRect(int x, int y, int w, int h, ColorRGBA color, bool isStroke, float strokeSize, float r) {
		drawRect(x, y, w, h, color, isStroke, strokeSize, r, r, r, r);
	}

	void UIRenderer::drawRect(int x, int y, int w, int h, ColorRGBA color, bool isStroke, float strokeSize, float topLeft, float topRight, float bottomRight, float bottomLeft) {

		NVGpaint shadowPaint = nvgBoxGradient(_context, x, y + 2, w, h, topLeft * 2, 10, nvgRGBA(0, 0, 0, 128), nvgRGBA(0, 0, 0, 0));
		nvgBeginPath(_context);
		nvgRect(_context, x - 10, y - 10, w + 20, h + 30);
		nvgRoundedRect(_context, x, y, w, h, topLeft);
		nvgPathWinding(_context, NVG_HOLE);
		nvgFillPaint(_context, shadowPaint);
		nvgFill(_context);

		nvgBeginPath(_context);
		if (topLeft == topRight == bottomRight == bottomLeft) {
			nvgRoundedRect(_context, (float)x, (float)y, (float)w, (float)h, (float)topLeft);
		} else {
			nvgRoundedRectVarying(_context, (float)x, (float)y, (float)w, (float)h, (float)topLeft, (float)topRight, (float)bottomRight, (float)bottomLeft);
		}
		NVGcolor c = nvgRGBAf(color.r, color.g, color.b, 1.0f);
		if (isStroke) {
			nvgStrokeWidth(_context, strokeSize);
			nvgStrokeColor(_context, c);
			nvgStroke(_context);
		} else {
			nvgFillColor(_context, c);
			nvgFill(_context);
		}
	}

	void UIRenderer::drawRect(int x, int y, int w, int h, const ColorRGBA& color, const Border& border, const DropShadow& shadow) {
		drawRect(x, y, w, h, makeColorPaint(color), border, shadow);
	}

	void UIRenderer::drawRect(int x, int y, int w, int h, const Paint& bgPaint, const Border& border, const DropShadow& shadow) {
		if (shadow.size > 0.0f) {
			NVGpaint shadowPaint = nvgBoxGradient(_context, x, y, w, h, border.radius.topLeft, shadow.size * 2.0f, nvgRGBAf(shadow.color.r, shadow.color.g, shadow.color.b, shadow.color.a), nvgRGBAf(shadow.color.r, shadow.color.g, shadow.color.b, 0));
			nvgBeginPath(_context);
			nvgRect(_context, x - shadow.size, y - shadow.size, w + shadow.size * 2.0f, h + shadow.size * 2.0f);
			nvgRoundedRect(_context, x, y, w, h, border.radius.topLeft + 2.0f);
			nvgPathWinding(_context, NVG_HOLE);
			nvgFillPaint(_context, shadowPaint);
			nvgFill(_context);
		}

		if (border.thickness > 0.0f) {
			nvgBeginPath(_context);
			nvgRoundedRectVarying(_context, (float)x, (float)y, (float)w, (float)h, (float)border.radius.topLeft, (float)border.radius.topRight, (float)border.radius.bottomRight, (float)border.radius.bottomLeft);
			NVGcolor c = nvgRGBAf(border.color.r, border.color.g, border.color.b, 1.0f);
			nvgStrokeWidth(_context, border.thickness + 0.5f);
			nvgStrokeColor(_context, c);
			nvgStroke(_context);
		}

		nvgBeginPath(_context);
		nvgRoundedRectVarying(_context, (float)x, (float)y, (float)w, (float)h, (float)border.radius.topLeft, (float)border.radius.topRight, (float)border.radius.bottomRight, (float)border.radius.bottomLeft);

		if (bgPaint.mode == PaintMode::Color) {
			NVGcolor c = nvgRGBAf(bgPaint.basicColor.r, bgPaint.basicColor.g, bgPaint.basicColor.b, 1.0f);
			nvgFillColor(_context, c);
		} else if (bgPaint.mode == PaintMode::LinearGradient) {
			NVGpaint nPaint;
			if (bgPaint.linearVertical) {
				nPaint = nvgLinearGradient(_context, x, y, x, y + h, makeNVGColor(bgPaint.startColor), makeNVGColor(bgPaint.endColor));
			} else {
				nPaint = nvgLinearGradient(_context, x, y, x + w, y, makeNVGColor(bgPaint.startColor), makeNVGColor(bgPaint.endColor));
			}
			nvgFillPaint(_context, nPaint);
		} else if (bgPaint.mode == PaintMode::RadialGradient) {

		}

		nvgFill(_context);
	}

	int UIRenderer::beginClip(int x, int y, int width, int height) {
		nvgSave(_context);
		nvgIntersectScissor(_context, x, y, width, height);
		return 0;
	}

	void UIRenderer::endClip(int id) {
		nvgRestore(_context);
	}

	void UIRenderer::clearClipping() {
		_nextClipId = 0;
	}
}
