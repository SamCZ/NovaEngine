#ifndef UI_RENDERER_H
#define UI_RENDERER_H

#include <map>
#include <string>

#include "NanoVG/nanovg.h"
#include "Engine/Render/Display.h"
#include "Engine/Render/Texture2D.h"
#include "Engine/Utils/ColorRGBA.h"

namespace NovaEngine {
	enum class Align {
		Baseline,
		Bottom,
		Center,
		Left,
		Right,
		Top
	};

	enum class PaintMode {
		LinearGradient,
		RadialGradient,
		Color
	};

	struct Paint {
		PaintMode mode;

		ColorRGBA basicColor;

		bool linearVertical;
		ColorRGBA startColor;
		ColorRGBA endColor;
	};

	struct BorderRadius {
		float topLeft = 0;
		float topRight = 0;
		float bottomRight = 0;
		float bottomLeft = 0;
	};

	struct Border {
		float thickness = 0;
		ColorRGBA color = makeRGBA(0, 0, 0, 0);
		BorderRadius radius;
	};

	struct DropShadow {
		float size;
		ColorRGBA color;
	};

	struct ClipData {
		ClipData* _parent;
		int x;
		int y;
		int width;
		int height;
	};

	char* cpToUTF8(int cp, char* str);

	BorderRadius makeRadius(float topLeft, float topRight, float bottomRight, float bottomLeft);
	BorderRadius makeRadius(float radius);
	Border makeBorder(float thickness, const ColorRGBA& color, const BorderRadius& radius);
	Border makeBorder(const BorderRadius& radius);
	Border makeBorder();
	DropShadow makeShadow(float size, const ColorRGBA& color);
	DropShadow makeShadow();

	Paint makeColorPaint(ColorRGBA color);
	Paint makeLinearGradientPaint(bool vertical, const ColorRGBA& startColor, const ColorRGBA& endColor);
	Paint makeRadialGradientPaint();

	NVGcolor makeNVGColor(const ColorRGBA& color);

	class UIRenderer {
	private:
		NVGcontext* _context;
		Display* _display;
		std::map<GLuint, int> _textureNVGStore;

		int _nextClipId;
		float* _textBounds;
		std::string _selectedFont;
	public:
		UIRenderer(Display* display);
		~UIRenderer();

		void begin();
		void end();

		Display* getDisplay();
		NVGcontext* getContext();

		void setFont(std::string font);

		void drawImage(Texture2D* tex, int x, int y, int width, int height, int borderRadius);
		int getTextureNVGHandle(Texture2D* tex);

		void drawText(std::string text, int x, int y, float fontSize, ColorRGBA color, Align alignX = Align::Left, Align alignY = Align::Top);

		float getTextSize(std::string text, int fontSize, int* textHeight);
		
		void drawRect(int x, int y, int w, int h, ColorRGBA color, bool isStroke);
		void drawRect(int x, int y, int w, int h, ColorRGBA color, bool isStroke, float strokeSize);
		void drawRect(int x, int y, int w, int h, ColorRGBA color, bool isStroke, float strokeSize, float r);
		void drawRect(int x, int y, int w, int h, ColorRGBA color, bool isStroke, float strokeSize, float topLeft, float topRight, float bottomRight, float bottomLeft);

		void drawRect(int x, int y, int w, int h, const ColorRGBA& color, const Border& border = makeBorder(), const DropShadow& shadow = makeShadow());
		void drawRect(int x, int y, int w, int h, const Paint& bgPaint, const Border& border = makeBorder(), const DropShadow& shadow = makeShadow());

		int beginClip(int x, int y, int width, int height);
		void endClip(int id);
		void clearClipping();
	};
}

#endif // !UI_RENDERER_H