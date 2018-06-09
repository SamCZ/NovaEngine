#include "Engine/Utils/FileTypes.h"
#include "Engine/Utils/File.h"
#include <vector>
#include <map>
#include "Engine/Utils/File.h"

namespace NovaEngine {
	static std::map<FileType, std::vector<std::string>> FileMap;
	static bool FileMapInitialized = false;
	
	
	FileType getFileType(const File& file) {
		//exp == "png" || exp == "jpg" || exp == "jpeg" || exp == "gif" || exp == "tiff" || exp == "tif" || exp == "tga"
		if (!FileMapInitialized) {
			FileMap[FileType::Texture].push_back("png");
			FileMap[FileType::Texture].push_back("jpg");
			FileMap[FileType::Texture].push_back("jpeg");
			FileMap[FileType::Texture].push_back("gif");
			FileMap[FileType::Texture].push_back("tiff");
			FileMap[FileType::Texture].push_back("tif");
			FileMap[FileType::Texture].push_back("tga");

			FileMap[FileType::Texture].push_back("r16");
			FileMap[FileType::Texture].push_back("r32");

			FileMap[FileType::TextDocument].push_back("txt");

			FileMap[FileType::Script].push_back("nsc");

			FileMap[FileType::Object3D].push_back("obj");
			FileMap[FileType::Object3D].push_back("fbx");
			FileMap[FileType::Object3D].push_back("stl");
			FileMap[FileType::Object3D].push_back("dae");
			FileMap[FileType::Object3D].push_back("3ds");
			FileMap[FileType::Object3D].push_back("c4d");

			FileMap[FileType::Material].push_back("mat");

			FileMap[FileType::Hidden].push_back("meta");
			FileMap[FileType::Hidden].push_back("map");

			FileMap[FileType::Scene].push_back("scene");


			//FileMap[FileType::Object3D].push_back("");

			FileMapInitialized = true;
		}
		std::string e = file.getExtension();
		for (std::map<FileType, std::vector<std::string>>::iterator i = FileMap.begin(); i != FileMap.end(); i++) {
			for (std::string ext : i->second) {
				if (ext == e) {
					return i->first;
				}
			}
		}
		return FileType::Unknown;
	}
}