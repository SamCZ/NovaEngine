#ifndef COLOR_BITMAP_H
#define COLOR_BITMAP_H

#include <array>
#include "Engine/Utils/ColorRGBA.h"

namespace NovaEngine {
	class ColorBitmap {
	public:
		ColorBitmap(int w, int h);
		ColorBitmap(unsigned char* pixels, int w, int h);
		~ColorBitmap();

		void clear();
		void setPixel(int x, int y, const ColorRGBA& col);
		void setPixel(int x, int y, int r, int g, int b);
		void setPixel(int x, int y, int channel, unsigned char val);
		void drawLine(int x0, int y0, int x1, int y1, const ColorRGBA& col);

		unsigned char* getPixels();

		ColorBitmap* scale(ColorBitmap* dest, int w, int h);

		void createBlur(int size);

		int getWidth() const;
		int getHeight() const;
	private:
		int _width;
		int _height;
		unsigned char* _colorDataBackground;
		unsigned char* _colorData;
		unsigned int _length;

		int bilinear_interpolation_read(double x, double y, int size, int colorIndex) const;

		std::array<unsigned int, 3> SampleBicubic(float u, float v);
		float CubicHermite(float A, float B, float C, float D, float t);

		unsigned char read(int x, int y, int colorIndex) const;
		unsigned char* read(int x, int y) const;
	};
}

#endif // !COLOR_BITMAP_H