#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include "Engine/Scene/Spatial.h"
#include "Engine/Render/Mesh/Mesh.h"
#include "Engine/Material/Material.h"
#include "Engine/Render/RenderState.h"
#include "Engine/Collisions/Ray.h"
#include "Engine/Collisions/Collidable.h"

namespace NovaEngine {

	class GameObject : public Spatial {
	public:
		GameObject();
		GameObject(std::string name);
		GameObject(std::string name, Material* material, Mesh* mesh);
		~GameObject();

		void setMesh(Mesh* mesh);
		virtual void setMaterial(Material* material) override;

		Mesh* getMesh();
		Material* getMaterial();

		int collideWithRay(Ray r, CollisionResults& results) override;

		void addPostProcessFlag(int flag);
		void removePostProcessFlag(int flag);
		bool hasPostProcessFlag(int flag);

		void setShadowMode(const ShadowMode& mode);
		ShadowMode getShadowMode() const;
	private:
		Mesh* _mesh = nullptr;
		Material* _material = nullptr;
		int _postprocessFlags;
		ShadowMode _shadowMode;
	};
}

#endif // !GAME_OBJECT_H