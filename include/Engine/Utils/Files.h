#ifndef E_FILES_H
#define E_FILES_H

#include <vector>
#include <string>
#include "Engine/Utils/File.h"
#include "Engine/Utils/json.hpp"

namespace NovaEngine {

	using jjson = nlohmann::json;

	class Files {
	public:
		static std::string readFile(File file, std::string lineSeparator = "");
		static std::vector<std::string> readLines(File file);

		static bool copyFile(const File& from, const File& to);

		static jjson loadJson(const File& file);
		static void saveJson(jjson& js, const File& file);
	};
}

#endif // !FILES_H