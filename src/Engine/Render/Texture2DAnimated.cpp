#include "Engine/Render/Texture2DAnimated.h"
#include <chrono>

namespace NovaEngine {

	double getMillis() {
		auto sec = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
		return static_cast<double>(sec.count());
	}

	class Seconds {
	public:
		inline Seconds() : mStart(std::chrono::system_clock::now()) {
			
		}

		// Self-contained API.
		inline void start() {
			start(std::chrono::system_clock::now());
		}
		inline void start(const double offset_seconds /* negative value to start at earlier than current titme */) {
			start();
			auto		sec(std::chrono::milliseconds((long)(offset_seconds*1000.0)));
			mStart += sec;
		}
		inline double elapsed() const {
			return elapsed(std::chrono::system_clock::now());
		}
		// API for when someone outside of me is tracking the current time.
		inline void start(const std::chrono::time_point<std::chrono::system_clock> &start_time) {
			mStart = start_time;
		}
		inline double elapsed(const std::chrono::time_point<std::chrono::system_clock> &end_time) const {
			auto diff = end_time - mStart;
			auto sec = std::chrono::duration_cast<std::chrono::milliseconds>(diff);
			return (static_cast<double>(sec.count()) / 1000.0);
		}

	private:
		std::chrono::time_point<std::chrono::system_clock> mStart;
	};

	const double DEFAULT_FPS = (7.0 / 60.0);

	Texture2DAnimated::Texture2DAnimated(int width, int height, const ImageFormat& format, std::vector<void*> pixels, std::vector<float> delays) : Texture2D(width, height, format, pixels) {
		_delays = delays;
		_frameCount = pixels.size();
		_currentFrame = 0;
		_timer = new Seconds();
		_nextTime = 0;
		_isRunning = false;
	}

	Texture2DAnimated::~Texture2DAnimated() {}

	void Texture2DAnimated::update() {
		const double elapsed = _timer->elapsed();
		if (elapsed >= _nextTime) {
			if (++_currentFrame >= _frameCount) {
				_currentFrame = 0;
			}
			restartTimer();
		} else if (_nextPlaybackSpeed != _playbackSpeed) {
			_playbackSpeed = _nextPlaybackSpeed;
			restartTimer();
		}
	}

	void Texture2DAnimated::restartTimer() {
		_nextTime = findFrameRate();

		// Modulate by current playback speed.
		/*if (_playbackSpeed <= 0.0f) {
			_nextTime = std::numeric_limits<float>::max();
		} else {
			_nextTime *= (1.0f / _playbackSpeed);
		}*/

		_timer->start();
	}

	float Texture2DAnimated::findFrameRate() {
		float delay = _delays[_currentFrame];
		if (delay > 0.00000001) return delay;
		return DEFAULT_FPS;
	}

	void Texture2DAnimated::start() {
		_currentFrame = 0;
		_nextTime = findFrameRate();
		_timer->start();
		_isRunning = true;
	}

	void Texture2DAnimated::stop() {
		_isRunning = false;
	}

	int Texture2DAnimated::getFrameCount() const {
		return _frameCount;
	}

	int Texture2DAnimated::getCurrentFrame() const {
		return _currentFrame;
	}

	float Texture2DAnimated::getDelay(int frame) const {
		return _delays[frame];
	}

	void Texture2DAnimated::setCurrentFrame(int frame) {
		_currentFrame = frame;
	}

	void Texture2DAnimated::setNextFrame() {
		_currentFrame++;
		if (_currentFrame > _frameCount-1) {
			_currentFrame = 0;
		}
	}

	bool Texture2DAnimated::isRunning() const {
		return _isRunning;
	}
}