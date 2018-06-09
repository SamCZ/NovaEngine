#ifndef SOUND_PLAYER_H
#define SOUND_PLAYER_H

#include <AL/alut.h>

namespace NovaEngine {
	class Sound;

	class SoundPlayer {
	private:
		ALuint _sourceId;
	public:
		SoundPlayer();
		~SoundPlayer();

		void setSound(Sound* sound);

		void play();
		void pause();
		void stop();

		bool isPlaying();
	};
}

#endif // !SOUND_PLAYER_H