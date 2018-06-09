#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <string>
#include "Engine/Scene/Camera.h"
#include "Engine/Scene/Spatial.h"
#include "Engine/Utils/ColorRGBA.h"

namespace NovaEngine {
	class ViewPort {
	private:
		std::string _name;
		Camera* _camera;
		ColorRGBA _backgroundColor;
		Spatial* _scene;
		bool _forcedSize;
	public:
		ViewPort(std::string name, Camera* camera);
		~ViewPort();

		Camera* getCamera();
		void setCamera(Camera* camera);

		void setBackgroundColor(ColorRGBA color);
		ColorRGBA& getBackgroundColor();

		void setScene(Spatial* scene);
		Spatial* getScene();

		int getWidth() const;
		int getHeight() const;

		void setName(std::string name);
		std::string getName() const;

		void setForcedSize(bool forced);
		bool isForcedSize() const;
	};
}

#endif // !VIEWPORT_H