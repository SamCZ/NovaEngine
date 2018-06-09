#ifndef SYSTEM_UTILS_H
#define SYSTEM_UTILS_H

#include <stdio.h>  /* defines FILENAME_MAX */
#ifdef _WIN32
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

namespace SYS {
#define SYSUTILS
	SYSUTILS std::string getCurrentDirectory();

	static std::string getCurrentDirectory() {
		char cCurrentPath[FILENAME_MAX];
		if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath))) {
			return "";
		}
		cCurrentPath[sizeof(cCurrentPath) - 1] = '\0';
		return std::string(cCurrentPath);
	}
}

#endif // !SYSTEM_UTILS_H