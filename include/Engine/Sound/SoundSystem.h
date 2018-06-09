#ifndef N_SOUND_SYSTEM_H
#define N_SOUND_SYSTEM_H

#include <vector>
#include <map>
#include "Engine/Sound/Sound.h"
#include "Engine/Sound/SoundPlayer.h"
#include "Engine/Utils/File.h"

#include "alc.h"
#include <AL/alut.h>

namespace NovaEngine {
	class SoundSystem {
	private:
		std::map<std::string, Sound*> _soundCache;

		void loadOGG(const char *fileName, std::vector<char> &buffer, ALenum &format, ALsizei &freq);
	public:
		~SoundSystem();
		SoundSystem();

		Sound* loadSound(const File& file);

		void update();
	};
}

#endif // !N_SOUND_SYSTEM_H