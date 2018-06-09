#include "Engine/Utils/ColorRGBA.h"
#include <iostream>
#include <sstream>

namespace NovaEngine {
	ColorRGBA makeRGB(int r, int g, int b) {
		ColorRGBA color;
		color.r = (float)r / 255.0f;
		color.g = (float)g / 255.0f;
		color.b = (float)b / 255.0f;
		color.a = 1.0f;
		return color;
	}

	ColorRGBA makeRGBA(int r, int g, int b, int a) {
		ColorRGBA color;
		color.r = (float)r / 255.0f;
		color.g = (float)g / 255.0f;
		color.b = (float)b / 255.0f;
		color.a = (float)a / 255.0f;
		return color;
	}

	ColorRGBA makeRGBf(float r, float g, float b) {
		ColorRGBA color;
		color.r = r;
		color.g = g;
		color.b = b;
		color.a = 1.0f;
		return color;
	}

	ColorRGBA makeRGBAf(float r, float g, float b, float a) {
		ColorRGBA color;
		color.r = r;
		color.g = g;
		color.b = b;
		color.a = a;
		return color;
	}

	ColorRGBA makeHEX(const char* hex) {
		int len = strlen(hex);
		if (len == 6) {
			std::stringstream ss;
			ss << std::hex << hex[0] << std::hex << hex[1];
			int r;
			ss >> r;
			ss.clear();
			ss << std::hex << hex[2] << std::hex << hex[3];
			int g;
			ss >> g;
			ss.clear();
			ss << std::hex << hex[4] << std::hex << hex[5];
			int b;
			ss >> b;
			ss.clear();

			return makeRGB(r, g, b);
		}
		return makeRGB(255, 255, 255);
	}
}