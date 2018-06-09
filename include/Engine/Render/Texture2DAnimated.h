#ifndef TEXTURE_2D_ANIMATED
#define TEXTURE_2D_ANIMATED

#include "Engine/Render/Texture2D.h"

namespace NovaEngine {
	class Seconds;

	class Texture2DAnimated : public Texture2D {
	private:
		bool _isRunning;
		int _frameCount;
		int _currentFrame;
		std::vector<float> _delays;
		Seconds* _timer;
		float _nextTime;
		float _playbackSpeed;
		float _nextPlaybackSpeed;

		void restartTimer();
		float findFrameRate();
		double _lastTime;
	public:
		Texture2DAnimated(int width, int height, const ImageFormat& format, std::vector<void*> pixels, std::vector<float> delays);
		~Texture2DAnimated();

		int getFrameCount() const;
		int getCurrentFrame() const;
		float getDelay(int frame) const;
		void setCurrentFrame(int frame);
		void setNextFrame();

		void update();

		void start();
		void stop();

		bool isRunning() const;
	};
}

#endif // !TEXTURE_2D_ANIMATED