#include "Editor/FileTree.h"

namespace NovaEngine {
	FileTree::~FileTree() {
		for (FileTree* tree : _childs) {
			delete tree;
		}
	}

	FileTree::FileTree(const File& path, bool isDirectory) : _path(path), _isDirectory(isDirectory) { }

	FileTree* FileTree::getOrCreateDir(const std::string& name) {
		for (FileTree* tree : _childs) {
			if (tree->getPath().getName() == name) {
				return tree;
			}
		}
		FileTree* newTree = new FileTree(File(getPath(), name), true);
		_childs.push_back(newTree);
		return newTree;
	}

	std::vector<FileTree*>& FileTree::getChilds() {
		return _childs;
	}

	bool FileTree::isDirectory() const {
		return _isDirectory;
	}

	FileData FileTree::addData(const File& path, const FileType& type, void* data) {
		if(dataExist(path)) return getData(path);
		FileData fData;
		fData.path = new File(path);
		fData.data = data;
		fData.type = type;
		_files.push_back(fData);
		return fData;
	}

	bool FileTree::dataExist(const File& path) {
		for (FileData& data : _files) {
			if (data.path->getPath() == path.getPath()) {
				return true;
			}
		}
		return false;
	}

	FileData FileTree::getData(const File & path) {
		for (FileData data : _files) {
			if (data.path->getPath() == path.getPath()) {
				return data;
			}
		}
		return FileData();
	}

	void FileTree::removeData(const File & path) {
		for (std::vector<FileData>::iterator data = _files.begin(); data != _files.end(); data++) {
			if (data->path->getPath() == path.getPath()) {
				_files.erase(data);
				return;
			}
		}
	}

	FileData* FileTree::findData(const File& find) {
		for (FileData& data : _files) {
			if (data.path->getPath() == find.getPath()) {
				return new FileData(data);
			}
		}
		for (FileTree* tree : _childs) {
			FileData* data = tree->findData(find);
			if (data != nullptr) {
				return data;
			}
		}
		return nullptr;
	}

	std::vector<FileData>& FileTree::getFiles() {
		return _files;
	}

	File FileTree::getPath() const {
		return _path;
	}
}