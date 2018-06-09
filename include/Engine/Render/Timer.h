#ifndef E_TIMER_H
#define E_TIMER_H

namespace NovaEngine {
	class Timer {
	public:
		Timer();

		void update();
		long getDelta();
		float getFrameRate();
		long getResolution();
		long getSystemTime();
	private:
		long _lastFrame;
		int _fpsCounter;
		long _lastFPS;
		long _fps;
		long _delta;
	};
}

#endif // !E_TIMER_H