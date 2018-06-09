#ifndef PARALLER_CAMERA_H
#define PARALLER_CAMERA_H

#include "Engine/Scene/Camera.h"

namespace NovaEngine {
	class ParallelCamera : public Camera {
	private:
		float _frustumSize;
		float _scaleX;
		float _scaleY;
	public:
		ParallelCamera(int width, int height);

		void update() override;

		void setFrustumSize(float frustumSize);
		void setScale(float x, float y);

		float getFrustumSize();
		float getScaleX();
		float getScaleY();
	};
}

#endif // !PARALLER_CAMERA_H