#ifndef MESH_UTILS_H
#define MESH_UTILS_H

#include "glm/glm.hpp"
#include "Engine/Render/Mesh/Mesh.h"

namespace NovaEngine {

	void transformMesh(Mesh* mesh, glm::mat4 mat);

}

#endif // !MESH_UTILS_H