#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <iterator>
#include <fstream>
#include <istream>
#include <streambuf>
#include <functional>
#include "glm/glm.hpp"
#include <glm/gtx/transform.hpp>

namespace NovaEngine {
	bool getCNName(std::string* pcName, std::string* userName);

	std::string formatF(float f);

	template<typename Out> void split(const std::string &s, char delim, Out result);
	std::vector<std::string> split(const std::string &s, char delim);

	std::string LatinToAscii(const std::string str);

	template<typename T, typename A>
	void swap(std::vector<T, A>& list, int i1, int i2) {
		T p1 = list[i1];
		list[i1] = list[i2];
		list[i2] = p1;
	}

	template<typename T, typename A>
	void arraycopy(std::vector<T, A> const& src, int srcPos, std::vector<T, A>& dest, int destPos, int length) {
		while (length > 0) {
			dest[destPos] = src[srcPos];
			srcPos++;
			destPos++;
			length--;
		}
	}

	template<typename T, typename A>
	void mergeSort(std::vector<T, A>& src, std::vector<T, A>& dest, int low, int high, int off, std::function<int(const T, const T)> compare_function) {
		int length = high - low;
		if (length < 7) {
			for (int i = low; i < high; i++) {
				for (int j = i; j > low && compare_function(dest[j - 1], dest[j]) > 0; j--) {
					swap(dest, j, j - 1);
				}
			}
			return;
		}
		int destLow = low;
		int destHigh = high;
		low += off;
		high += off;
		int mid = (low + high) >> 1;
		mergeSort(dest, src, low, mid, -off, compare_function);
		mergeSort(dest, src, mid, high, -off, compare_function);

		if (compare_function(src[mid - 1], src[mid]) <= 0) {
			arraycopy(src, low, dest, destLow, length);
			return;
		}

		// Merge sorted halves (now in src) into dest
		for (int i = destLow, p = low, q = mid; i < destHigh; i++) {
			if (q >= high || p < mid && compare_function(src[p], src[q]) <= 0)
				dest[i] = src[p++];
			else
				dest[i] = src[q++];
		}
	}

	template<typename T, typename A>
	void mergeSort(std::vector<T, A>& src, std::function<int(const T left, const T right)> compare_function) {
		std::vector<T, A> copyOfsrc(src);
		mergeSort<T>(src, copyOfsrc, 0, src.size(), 0, compare_function);
		src = copyOfsrc;
	}

	template <typename T> std::string toStr(const T& t) {
		std::ostringstream os;
		os << t;
		return os.str();
	}

	float lerp(float s, float e, float t);
	float blerp(float c00, float c10, float c01, float c11, float tx, float ty);

	int compareFloat(float f1, float f2);
	float round(float value, int places);
	glm::mat4 createMatrix(glm::vec3 pos, glm::vec3 rot, glm::vec3 scale);

	std::string readString(std::ifstream& stream);
	int readInt(std::ifstream& stream);
	char readChar(std::ifstream& stream);
	float readFloat(std::ifstream& stream);
	bool readBool(std::ifstream& stream);
	int* readIntArray(std::ifstream& stream, int* size);
	float* readFloatArray(std::ifstream& stream, int* size);
	glm::vec2 readVector2(std::ifstream& stream);
	glm::vec3 readVector3(std::ifstream& stream);
	glm::vec4 readVector4(std::ifstream& stream);




	void writeString(std::string str, std::ofstream& stream);
	void writeInt(int i, std::ofstream& stream);
	void writeChar(char c, std::ofstream& stream);
	void writeFloat(float f, std::ofstream& stream);
	void writeBool(bool b, std::ofstream& stream);
	void writeIntArray(int* arr, int size, std::ofstream& stream);
	void writeFloatArray(float* arr, int size, std::ofstream& stream);
	void writeVector2(glm::vec2 vec, std::ofstream& stream);
	void writeVector3(glm::vec3 vec, std::ofstream& stream);
	void writeVector4(glm::vec4 vec, std::ofstream& stream);
}

#endif // !UTILS_H
