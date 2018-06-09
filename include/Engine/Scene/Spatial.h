#ifndef SPATIAL_H
#define SPATIAL_H

#include <vector>
#include <string>

#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "Engine/Scene/Animator.h"

#include "Engine/Collisions/Plane.h"
#include "Engine/Collisions/BoundingBox.h"
#include "Engine/Collisions/Ray.h"

#include "Engine/Scene/Components.h"

namespace NovaEngine {

	class File;
	class Light;

	enum class RenderOrder {
		Opaque,
		Trasparent,
		Sky,
		Gui
	};

	enum class SpatialType {
		Node, GameObject
	};

	enum class ShadowMode {
		None,
		Cast,
		Receive,
		CastAndReceive
	};

	class Material;

	class Spatial {
	public:
		Spatial();
		Spatial(std::string name);
		~Spatial();

		void setLocation(const glm::vec3& location);
		void setRotation(const glm::vec3& rotation);
		void setScale(const glm::vec3& scale);

		void setLocation(float x, float y, float z);
		virtual void setRotation(float x, float y, float z);
		void setScale(float x, float y, float z);

		glm::vec3& getLocation();
		glm::vec3& getRotation();
		glm::vec3& getScale();

		glm::vec3 getWorldLocation();
		glm::vec3 getWorldRotation();
		glm::vec3 getWorldScale();

		glm::vec3 getParentLocation();
		glm::vec3 getParentRotation();
		glm::vec3 getParentScale();

		void addChild(Spatial* spatial);
		void removeChild(Spatial* spatial);
		std::vector<Spatial*>& getChilds();
		int getChildCount() const;
		void removeAllChildren();
		bool isChildExist(Spatial* spatial);

		Spatial* clone(bool deep = false);
		
		void setParent(Spatial* spatial);
		Spatial* getParent();

		Spatial* operator[](int index);

		void setRenderOrder(RenderOrder order);
		RenderOrder getRenderOrder() const;

		glm::mat4 getModelMatrix(bool useWorld = true);

		void setType(SpatialType type);
		SpatialType getType() const;

		void setVisible(bool visible);
		bool& isVisible();
		bool isVisibleInTree();

		virtual void setMaterial(Material* material);
		virtual void preRender();

		virtual void update();

		void setName(std::string name);
		std::string getName() const;

		void setAnimator(Animator* animator);
		Animator* getAnimator();

		void addLight(Light* light);
		void removeLight(Light* light);
		std::vector<Light*>& getLights();
		bool isLightNode() const;

		virtual int collideWithRay(Ray r, CollisionResults& results);

		File* getFileSource();
		void setFileSource(const File& file);

		void setTemp(bool temp);
		bool isTemp() const;

		void addComponent(Component* component);
		bool removeComponent(Component* component);
		std::vector<Component*>& getComponents();

		void setUseParentTransform(bool useParent);
		bool isUsingParentTransform() const;

		Spatial* find(const std::string& name);
	protected:
		std::string _name;
		glm::vec3 _location;
		glm::vec3 _rotation;
		glm::vec3 _scale;
		bool _visible = true;
		Animator* _animator;
		std::vector<Light*> _lights;
		bool _isLightNode;
		bool _isTemp;
		File* _fileSource = nullptr;
		std::vector<Component*> _components;
		bool _useParentTransform;

		SpatialType _type = SpatialType::Node;

		RenderOrder _renderOrder = RenderOrder::Opaque;
		Spatial* _parent = nullptr;

		std::vector<Spatial*> _childs;
	};
}

#endif // !SPATIAL_H