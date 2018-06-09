#include "Engine/Utils/ColorBitmap.h"

#include <string.h> // memcpy
#include <stdlib.h> //realloc
#include <stdio.h>      /* printf */
#include <math.h>       /* fabs */
#include <iostream>

namespace NovaEngine {

	ColorBitmap::ColorBitmap(int w, int h) {
		_width = w;
		_height = h;
		_length = w * h * 4;
		_colorData = new unsigned char[_length];
		_colorDataBackground = new unsigned char[_length];

		for (int x = 0; x < w; x++) {
			for (int y = 0; y < h; y++) {
				int index = (x + y * w) * 4;
				_colorDataBackground[index + 0] = 0;
				_colorDataBackground[index + 1] = 0;
				_colorDataBackground[index + 2] = 0;
				_colorDataBackground[index + 3] = 255;
			}
		}
		this->clear();
	}

	ColorBitmap::ColorBitmap(unsigned char* pixels, int w, int h) : ColorBitmap(w, h) {
		for (int i = 0; i < _length; i++) {
			_colorData[i] = pixels[i];
		}
	}

	ColorBitmap::~ColorBitmap() {
		delete[] _colorData;
		delete[] _colorDataBackground;
	}

	void ColorBitmap::clear() {
		memcpy(_colorData, _colorDataBackground, _length * sizeof(unsigned char));
	}

	void ColorBitmap::setPixel(int x, int y, const ColorRGBA& col) {
		if (x < 0 || y < 0 || x > _width - 1 || y > _height - 1) return;
		int index = (x + (y * _width)) * 4;
		_colorData[index + 0] = (unsigned char)(col.r * 255.0f) & 255;
		_colorData[index + 1] = (unsigned char)(col.g * 255.0f) & 255;
		_colorData[index + 2] = (unsigned char)(col.b * 255.0f) & 255;
		_colorData[index + 3] = (unsigned char)(col.a * 255.0f) & 255;

		/*int r = (int)(col.r * 255.0f) & 255;
		int g = (int)(col.g * 255.0f) & 255;
		int b = (int)(col.b * 255.0f) & 255;
		int a = (int)(col.a * 255.0f) & 255;

		_pixelData[x + (y * _width)] = (int)(((a & 255) << 24) | ((r & 255) << 16) | ((g & 255) << 8) | (b & 255));*/
	}

	void ColorBitmap::setPixel(int x, int y, int r, int g, int b) {
		if (x < 0 || y < 0 || x > _width - 1 || y > _height - 1) return;
		int index = (x + (y * _width)) * 4;
		_colorData[index + 0] = r & 255;
		_colorData[index + 1] = g & 255;
		_colorData[index + 2] = b & 255;
		_colorData[index + 3] = 255;
	}

	void ColorBitmap::setPixel(int x, int y, int channel, unsigned char val) {
		if (x < 0 || y < 0 || x > _width - 1 || y > _height - 1) return;
		int index = (x + (y * _width)) * 4;
		_colorData[index + channel] = val;
	}

	void ColorBitmap::drawLine(int x1, int y1, int x2, int y2, const ColorRGBA& col) {
		int x, y, dx, dy, dx1, dy1, px, py, xe, ye, i;
		dx = x2 - x1;
		dy = y2 - y1;
		dx1 = fabs((float)dx);
		dy1 = fabs((float)dy);
		px = 2 * dy1 - dx1;
		py = 2 * dx1 - dy1;
		if (dy1 <= dx1) {
			if (dx >= 0) {
				x = x1;
				y = y1;
				xe = x2;
			} else {
				x = x2;
				y = y2;
				xe = x1;
			}
			setPixel(x, y, col);
			for (i = 0; x < xe; i++) {
				x = x + 1;
				if (px<0) {
					px = px + 2 * dy1;
				} else {
					if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0)) {
						y = y + 1;
					} else {
						y = y - 1;
					}
					px = px + 2 * (dy1 - dx1);
				}
				setPixel(x, y, col);
			}
		} else {
			if (dy >= 0) {
				x = x1;
				y = y1;
				ye = y2;
			} else {
				x = x2;
				y = y2;
				ye = y1;
			}
			setPixel(x, y, col);
			for (i = 0; y < ye; i++) {
				y = y + 1;
				if (py <= 0) {
					py = py + 2 * dx1;
				} else {
					if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0)) {
						x = x + 1;
					} else {
						x = x - 1;
					}
					py = py + 2 * (dx1 - dy1);
				}
				setPixel(x, y, col);
			}
		}
	}

	unsigned char* ColorBitmap::getPixels() {
		return _colorData;
	}

	ColorBitmap* ColorBitmap::scale(ColorBitmap* dest, int w, int h) {
		if (dest == nullptr) {
			dest = new ColorBitmap(w, h);
		}

		for (int y = 0; y < h; ++y) {
			float v = float(y) / float(h - 1);
			for (int x = 0; x < w; ++x) {
				float u = float(x) / float(w - 1);
				std::array<unsigned int, 3> sample = SampleBicubic(u, v);
				dest->setPixel(x, y, sample[0], sample[1], sample[2]);
			}
		}

		return dest;
	}

	void ColorBitmap::createBlur(int size) {
		int r, g, b;
		for (int x = 0; x < _width; x++) {
			for (int y = 0; y < _height; y++) {
				r = bilinear_interpolation_read(x, y, size, 0);
				g = bilinear_interpolation_read(x, y, size, 1);
				b = bilinear_interpolation_read(x, y, size, 2);
				setPixel(x, y, r, g, b);
			}
		}
	}

	int ColorBitmap::getWidth() const {
		return _width;
	}

	int ColorBitmap::getHeight() const {
		return _height;
	}

	int ColorBitmap::bilinear_interpolation_read(double x, double y, int size, int colorIndex) const {
		if (size <= 0) {
			return read(x, y, colorIndex);
		}
		int color = 0;
		int a = 0;
		for (int i = -size; i < size; i++) {
			for (int j = -size; j < size; j++) {
				color += read(x + i, y + j, colorIndex);
				a++;
			}
		}
		color /= a;
		return color;
	}

	std::array<unsigned int, 3> ColorBitmap::SampleBicubic(float u, float v) {
		// calculate coordinates -> also need to offset by half a pixel to keep image from shifting down and left half a pixel
		float x = (u * _width) - 0.5;
		int xint = int(x);
		float xfract = x - floor(x);

		float y = (v * _height) - 0.5;
		int yint = int(y);
		float yfract = y - floor(y);

		auto p00 = read(xint - 1, yint - 1);
		auto p10 = read(xint + 0, yint - 1);
		auto p20 = read(xint + 1, yint - 1);
		auto p30 = read(xint + 2, yint - 1);

		// 2nd row
		auto p01 = read(xint - 1, yint + 0);
		auto p11 = read(xint + 0, yint + 0);
		auto p21 = read(xint + 1, yint + 0);
		auto p31 = read(xint + 2, yint + 0);

		// 3rd row
		auto p02 = read(xint - 1, yint + 1);
		auto p12 = read(xint + 0, yint + 1);
		auto p22 = read(xint + 1, yint + 1);
		auto p32 = read(xint + 2, yint + 1);

		// 4th row
		auto p03 = read(xint - 1, yint + 2);
		auto p13 = read(xint + 0, yint + 2);
		auto p23 = read(xint + 1, yint + 2);
		auto p33 = read(xint + 2, yint + 2);

		// interpolate bi-cubically!
		// Clamp the values since the curve can put the value below 0 or above 255
		std::array<unsigned int, 3> ret;
		for (int i = 0; i < 3; ++i) {
			float col0 = CubicHermite(p00[i], p10[i], p20[i], p30[i], xfract);
			float col1 = CubicHermite(p01[i], p11[i], p21[i], p31[i], xfract);
			float col2 = CubicHermite(p02[i], p12[i], p22[i], p32[i], xfract);
			float col3 = CubicHermite(p03[i], p13[i], p23[i], p33[i], xfract);
			float value = CubicHermite(col0, col1, col2, col3, yfract);
			if (value < 0) value = 0;
			if (value > 255) value = 255;
			ret[i] = (unsigned char)value;
		}
		delete[] p00;
		delete[] p10;
		delete[] p20;
		delete[] p30;

		delete[] p01;
		delete[] p11;
		delete[] p21;
		delete[] p31;

		delete[] p02;
		delete[] p12;
		delete[] p22;
		delete[] p32;

		delete[] p03;
		delete[] p13;
		delete[] p23;
		delete[] p33;

		return ret;
	}

	float ColorBitmap::CubicHermite(float A, float B, float C, float D, float t) {
		float a = -A / 2.0f + (3.0f*B) / 2.0f - (3.0f*C) / 2.0f + D / 2.0f;
		float b = A - (5.0f*B) / 2.0f + 2.0f*C - D / 2.0f;
		float c = -A / 2.0f + C / 2.0f;
		float d = B;

		return a*t*t*t + b*t*t + c*t + d;
	}

	unsigned char ColorBitmap::read(int x, int y, int colorIndex) const {
		if (x < 0 || y < 0 || x > _width - 1 || y > _height - 1) return 0;
		int index = (x + (y * _width)) * 4;
		return _colorData[index + colorIndex];
	}

	unsigned char * ColorBitmap::read(int x, int y) const {
		return new unsigned char[3]{ read(x, y, 0), read(x, y, 1), read(x, y, 2) };
	}
}