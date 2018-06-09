#include "Engine/Render/Timer.h"
#include "SDL.h"

namespace NovaEngine {

	Timer::Timer() {
		_lastFPS = getSystemTime();
		update();
	}

	void Timer::update() {
		if (getSystemTime() - _lastFPS > 1000) {
			_fps = _fpsCounter;
			_fpsCounter = 0;
			_lastFPS += 1000;
		}
		_fpsCounter++;

		long time = getSystemTime();
		long delta = time - _lastFrame;
		_lastFrame = time;
		_delta = delta;
	}

	long Timer::getDelta() {
		return _delta;
	}

	float Timer::getFrameRate() {
		return _fps;
	}

	long Timer::getResolution() {
		return 1000;
	}

	long Timer::getSystemTime() {
		return (long)(SDL_GetTicks() * 1000L / getResolution());
	}
}