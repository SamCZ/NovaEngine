#ifndef FILE_TREE_H
#define FILE_TREE_H

#include <string>
#include <vector>
#include "Engine/Utils/File.h"
#include "Engine/Utils/FileTypes.h"

namespace NovaEngine {

	typedef struct FILE_DATA_t {
		File *path = nullptr;
		void* data = nullptr;
		FileType type = FileType::Unknown;
	} FileData;

	class FileTree {
	private:
		std::vector<FileTree*> _childs;
		std::vector<FileData> _files;
		const File _path;
		bool _isDirectory;
	public:
		~FileTree();
		FileTree(const File& path, bool isDirectory);

		FileTree* getOrCreateDir(const std::string& name);
		std::vector<FileTree*>& getChilds();
		bool isDirectory() const;

		FileData addData(const File& path, const FileType& type, void* data);
		bool dataExist(const File& path);
		FileData getData(const File& path);
		void removeData(const File& path);

		FileData* findData(const File& find);

		std::vector<FileData>& getFiles();

		File getPath() const;
	};

	typedef struct FILE_TREE_QUEUE_ITEM_t {
		FileTree* baseTree;
		FileTree* tree = nullptr;
		FileData data;
		bool useData = false;
	} FileTreeQueueItem;
}

#endif // !FILE_TREE_H