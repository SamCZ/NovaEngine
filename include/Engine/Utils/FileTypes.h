#ifndef FILE_TYPES_H
#define FILE_TYPES_H

#include <string>

namespace NovaEngine {
	enum class FileType {
		Texture,
		TextDocument,
		Object3D,
		Script,
		Material,
		Scene,
		Unknown,
		Hidden,
		Unsupported,
		Raw16,
		Raw32
	};

	class File;

	FileType getFileType(const File& file);
}

#endif // !FILE_TYPES_H