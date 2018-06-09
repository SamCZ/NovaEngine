#include <Windows.h>

#include "boost/locale.hpp"
#include "boost/filesystem.hpp"

#include "Editor/Editor.h"
//#include "Editor/EditorNew.h"

int run() {
	std::locale::global(boost::locale::generator().generate(""));
	boost::filesystem::path::imbue(std::locale());
	NovaEngine::Engine* game = new Editor();
	game->start();
	delete game;
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	AllocConsole();
	freopen("conin$", "r", stdin);
	freopen("conout$", "w", stdout);
	freopen("conout$", "w", stderr);
	return run();
}