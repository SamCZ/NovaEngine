#ifndef DISPLAY_H
#define DISPLAY_H

#include <map>
#include <string>
#include <functional>

#include "SDL.h"
#include "Engine/Input/InputManager.h"
#include "Engine/Render/CursorType.h"

#ifdef _WIN32
#include <Windows.h>
#endif // _WIN32

namespace NovaEngine {
	enum class DisplayState {
		Normal, Maximized
	};

	class Display {
	public:
		Display();
		~Display();

		void create();
		void updateEvents();
		void update();
		void destroy();

		void setAutoSize();
		void setSize(int width, int height);
		int getWidth() const;
		int getHeight() const;

		void setResizable(bool resizable);
		bool isResizable() const;

		void stop();
		bool isRunning() const;

		int getMonitorCount() const;
		void getMonitorProperties(int monitor, int* w, int* h, int* refreshRate) const;

		float getPixelRatio();

		#ifdef _WIN32
		HWND getWindowHandle();
		#endif // _WIN32

		void setWindowState(DisplayState state);
		DisplayState getDisplayState() const;

		void setInputManager(InputManager* inputManager);
		void setResizeListener(std::function<void(int w, int h)> listener);
		void setEventListener(std::function<void(SDL_Event&)> listener);
		void setFileDropListener(std::function<void(std::string file, int x, int y)> listener);

		void setTitleAdditional(const std::string& title);

		void setMouseGrabbed(bool state);

		SDL_Window* getWindow() const;

		void setCursor(const CursorType& cursor);
	private:
		SDL_Window* _window = nullptr;
		SDL_GLContext _glContext;
		InputManager* _inputManager = nullptr;
		std::map<int, SDL_Cursor*> _loadedCursors;
		DisplayState _state;
		std::string _title;
		int _width = 0;
		int _height = 0;
		bool _created;
		bool _resizable;
		bool _isRunning;

		std::function<void(int w, int h)> _resizeListener;
		std::function<void(SDL_Event&)> _eventListener;
		std::function<void(std::string file, int x, int y)> _fileDropListener;
	};
}

#endif // !DISPLAY_H