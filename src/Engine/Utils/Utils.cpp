#include "Engine/Utils/Utils.h"
#include "glm/glm.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <Windows.h>

namespace NovaEngine {
	bool getCNName(std::string* pcName, std::string* userName) {
		if (pcName) {
			*pcName = std::string(getenv("COMPUTERNAME"));
		}
		if (userName) {
			*userName = std::string(getenv("USERNAME"));
		}
		return true;
	}

	std::string LatinToAscii(const std::string str) {
		std::string s;
		for (char c : str) {
			switch (c) {
				case 'ì':
				s += 'e';
				break;

				case 'š':
				s += 's';
				break;

				case 'è':
				s += 'c';
				break;

				case 'ø':
				s += 'r';
				break;

				case 'ž':
				s += 'z';
				break;

				case 'ý':
				s += 'y';
				break;

				case 'á':
				s += 'a';
				break;

				case 'í':
				s += 'i';
				break;

				case 'é':
				s += 'e';
				break;

				default:
				s += c;
				break;
			}
		}
		return s;
		/*const char* utf8 = str.c_str();
		// convert multibyte UTF-8 to wide string UTF-16
		int length = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)utf8, -1, NULL, 0);
		if (length > 0) {
			wchar_t* wide = new wchar_t[length];
			MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)utf8, -1, wide, length);

			// convert it to ANSI, use setlocale() to set your locale, if not set
			size_t convertedChars = 0;
			char* ansi = new char[length];
			wcstombs_s(&convertedChars, ansi, length, wide, _TRUNCATE);
			return std::string(ansi);
		}
		return "";*/
	}

	std::string formatF(float f) {
		std::stringstream stream;
		stream << std::fixed << std::setprecision(2) << f;
		return stream.str();
	}

	template<typename Out> void split(const std::string &s, char delim, Out result) {
		std::stringstream ss;
		ss.str(s);
		std::string item;
		while (std::getline(ss, item, delim)) {
			*(result++) = item;
		}
	}

	std::vector<std::string> split(const std::string &s, char delim) {
		std::vector<std::string> elems;
		split(s, delim, std::back_inserter(elems));
		return elems;
	}

	int compareFloat(float f1, float f2) {
		if (f1 < f2) {
			return -1;
		}
		if (f1 > f2) {
			return 1;
		}

		return f1 == f2 ? 0 : (f1 < f2 ? -1 : 1);
	}

	float round(float value, int places) {
		if (places < 0) return 0;

		long factor = (long)glm::pow(10, places);
		value = value * factor;
		long tmp = glm::round(value);
		return (float)tmp / factor;
	}

	float lerp(float s, float e, float t) { return s + (e - s)*t; }
	float blerp(float c00, float c10, float c01, float c11, float tx, float ty) {
		return lerp(lerp(c00, c10, tx), lerp(c01, c11, tx), ty);
	}

	glm::mat4 createMatrix(glm::vec3 pos, glm::vec3 rot, glm::vec3 scale) {
		glm::mat4 rotation = glm::mat4();

		rotation *= glm::rotate(glm::radians(rot.x), glm::vec3(1, 0, 0));
		rotation *= glm::rotate(glm::radians(rot.y), glm::vec3(0, 1, 0));
		rotation *= glm::rotate(glm::radians(rot.z), glm::vec3(0, 0, 1));

		return (glm::translate(pos) * rotation) * glm::scale(scale);
	}

	std::string readString(std::ifstream& stream) {
		std::string str;
		int length = readInt(stream);
		for (int i = 0; i < length; i++) {
			str.append(1, readChar(stream));
		}
		return str;
	}

	int readInt(std::ifstream& stream) {
		int i = -1;
		stream.read((char*)&i, sizeof(int));
		return i;
	}

	char readChar(std::ifstream & stream) {
		char c = -1;
		stream.read((char*)&c, sizeof(char));
		return c;
	}

	float readFloat(std::ifstream& stream) {
		float f = -1;
		stream.read((char*)&f, sizeof(float));
		return f;
	}

	bool readBool(std::ifstream& stream) {
		short s = -1;
		stream.read((char*)&s, sizeof(short));
		return s == 1;
	}

	int* readIntArray(std::ifstream& stream, int* size) {
		int s = readInt(stream);
		size = &s;
		int* arr = new int[s];
		for (int i = 0; i < s; i++) {
			arr[i] = readInt(stream);
		}
		return arr;
	}

	float* readFloatArray(std::ifstream& stream, int * size) {
		int s = readInt(stream);
		size = &s;
		float* arr = new float[s];
		for (int i = 0; i < s; i++) {
			arr[i] = readFloat(stream);
		}
		return arr;
	}

	glm::vec2 readVector2(std::ifstream& stream) {
		return glm::vec2(readFloat(stream), readFloat(stream));
	}

	glm::vec3 readVector3(std::ifstream & stream) {
		return glm::vec3(readFloat(stream), readFloat(stream), readFloat(stream));
	}

	glm::vec4 readVector4(std::ifstream& stream) {
		return glm::vec4(readFloat(stream), readFloat(stream), readFloat(stream), readFloat(stream));
	}

	void writeString(std::string str, std::ofstream& stream) {
		writeInt(str.length(), stream);
		for (int i = 0; i < str.length(); i++) {
			writeChar(str[i], stream);
		}
	}

	void writeInt(int i, std::ofstream & stream) {
		stream.write((char*)&i, sizeof(i));
	}

	void writeChar(char c, std::ofstream & stream) {
		stream.write((char*)&c, sizeof(c));
	}

	void writeFloat(float f, std::ofstream & stream) {
		stream.write((char*)&f, sizeof(f));
	}

	void writeBool(bool b, std::ofstream & stream) {
		short s = b ? 1 : 0;
		stream.write((char*)&s, sizeof(s));
	}

	void writeIntArray(int * arr, int size, std::ofstream & stream) {
		writeInt(size, stream);
		for (int i = 0; i < size; i++) {
			writeInt(arr[i], stream);
		}
	}

	void writeFloatArray(float * arr, int size, std::ofstream & stream) {
		writeInt(size, stream);
		for (int i = 0; i < size; i++) {
			writeFloat(arr[i], stream);
		}
	}

	void writeVector2(glm::vec2 vec, std::ofstream & stream) {
		writeFloat(vec.x, stream);
		writeFloat(vec.y, stream);
	}

	void writeVector3(glm::vec3 vec, std::ofstream & stream) {
		writeFloat(vec.x, stream);
		writeFloat(vec.y, stream);
		writeFloat(vec.z, stream);
	}

	void writeVector4(glm::vec4 vec, std::ofstream & stream) {
		writeFloat(vec.x, stream);
		writeFloat(vec.y, stream);
		writeFloat(vec.z, stream);
		writeFloat(vec.w, stream);
	}
}
