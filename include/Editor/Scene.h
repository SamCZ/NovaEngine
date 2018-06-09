#ifndef NSCENE_H
#define NSCENE_H

#include <string>
#include "Engine/Scene/Spatial.h"

namespace NovaEngine {
	class Scene {
	private:
		Spatial* _scene;
		std::string _filename;
	public:
		Scene(Spatial* scene, std::string filename);

		Spatial* getScene();
		std::string getFilename();
	};
}

#endif // !NSCENE_H