#ifndef E_FILE_H
#define E_FILE_H

#include <string>
#include <vector>

namespace NovaEngine {
	class File {
	public:
		~File();
		File();
		File(const char* file);
		File(std::string file);
		File(File parent, std::string file);

		std::string getPath() const;
		std::string getParent() const;
		File getParentFile() const;

		bool isExist() const;
		bool isDirectory() const;
		bool mkdirs() const;
		bool deleteFile() const;

		std::string getName() const;
		std::string getCleanName() const;
		std::string getExtension(bool lower = true) const;
		bool haveExtension(std::string ext);

		std::vector<File> listFiles(bool recursive = false);

		static std::string fixPath(std::string path, const char from = '\\', const char to = '/');
	private:
		std::string _rootPath;
	};

	std::ostream& operator<<(std::ostream& os, const File& obj);
	bool operator==(const File& left, const File& right);
	bool operator!=(const File& left, const File& right);
}

#endif // !E_FILE_H